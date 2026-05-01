#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

// --- Структуры ---
struct Bullet {
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

struct Enemy {
    sf::CircleShape shape;
    float speed;
    int hp;
};

// Вспомогательная функция для проверки столкновения двух кругов
bool circlesOverlap(const sf::Vector2f& aPos, float aRad,
                    const sf::Vector2f& bPos, float bRad) {
    sf::Vector2f diff = aPos - bPos;
    // Расстояние < сумма радиусов → пересечение
    return std::hypot(diff.x, diff.y) < (aRad + bRad);
}

int main() {
    const unsigned int WIDTH = 800;
    const unsigned int HEIGHT = 600;
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Isaac-like + Enemy");
    window.setFramerateLimit(60);

    // Игрок
    sf::CircleShape player(15.f);
    player.setFillColor(sf::Color::Green);
    player.setOrigin(player.getRadius(), player.getRadius());
    player.setPosition(WIDTH / 2.f, HEIGHT / 2.f);

    const float PLAYER_SPEED = 250.f;
    const float BULLET_SPEED = 400.f;
    const float SHOOT_COOLDOWN = 0.3f;
    float shootTimer = 0.f;

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;

    // Спавн первого врага
    Enemy e;
    e.shape.setRadius(20.f);
    e.shape.setFillColor(sf::Color::Red);
    e.shape.setOrigin(20.f, 20.f);
    e.shape.setPosition(100.f, 100.f);
    e.speed = 120.f;
    e.hp = 3;
    enemies.push_back(e);

    sf::Clock deltaClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        float dt = deltaClock.restart().asSeconds();
        shootTimer -= dt;

        // 1. Движение игрока (WASD)
        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W)) movement.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S)) movement.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A)) movement.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D)) movement.x += 1.f;

        if (movement.x != 0.f || movement.y != 0.f)
            movement /= std::hypot(movement.x, movement.y);

        player.move(movement * PLAYER_SPEED * dt);

        // Ограничение игрока
        float pr = player.getRadius();
        sf::Vector2f pPos = player.getPosition();
        pPos.x = std::clamp(pPos.x, pr, static_cast<float>(WIDTH - pr));
        pPos.y = std::clamp(pPos.y, pr, static_cast<float>(HEIGHT - pr));
        player.setPosition(pPos);

        // 2. Стрельба (Стрелки)
        sf::Vector2f shootDir(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    shootDir.y = -1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  shootDir.y =  1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  shootDir.x = -1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) shootDir.x =  1.f;

        if ((shootDir.x != 0.f || shootDir.y != 0.f) && shootTimer <= 0.f) {
            shootDir /= std::hypot(shootDir.x, shootDir.y);
            Bullet b;
            b.shape.setRadius(5.f);
            b.shape.setFillColor(sf::Color::Yellow);
            b.shape.setOrigin(5.f, 5.f);
            b.shape.setPosition(pPos + shootDir * (pr + 8.f));
            b.velocity = shootDir * BULLET_SPEED;
            bullets.push_back(b);
            shootTimer = SHOOT_COOLDOWN;
        }

        // 3. Обновление пуль
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            it->shape.move(it->velocity * dt);
            sf::FloatRect bnd = it->shape.getGlobalBounds();
            if (bnd.left + bnd.width < 0 || bnd.left > WIDTH ||
                bnd.top + bnd.height < 0 || bnd.top > HEIGHT) {
                it = bullets.erase(it);
            } else ++it;
        }

        // 4. Обновление врагов (ИИ преследования)
        pPos = player.getPosition(); // Обновляем позицию игрока для расчетов
        for (auto& enemy : enemies) {
            sf::Vector2f toPlayer = pPos - enemy.shape.getPosition();
            float dist = std::hypot(toPlayer.x, toPlayer.y);

            // Двигаемся только если дистанция > 1 пикселя (защита от деления на 0 и дрожания)
            if (dist > 1.f) {
                sf::Vector2f dir = toPlayer / dist; // Нормализация
                enemy.shape.move(dir * enemy.speed * dt);
            }

            // Столкновение Враг ↔ Игрок
            if (circlesOverlap(enemy.shape.getPosition(), enemy.shape.getRadius(),
                               pPos, player.getRadius())) {
                // Простой отброс игрока (knockback)
                sf::Vector2f pushDir = pPos - enemy.shape.getPosition();
                if (std::hypot(pushDir.x, pushDir.y) > 0) {
                    pushDir /= std::hypot(pushDir.x, pushDir.y);
                    player.move(pushDir * 4.f);
                }
            }
        }

        // 5. Столкновение Пули ↔ Враги
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            bool hit = false;
            for (auto eit = enemies.begin(); eit != enemies.end(); ) {
                if (circlesOverlap(it->shape.getPosition(), it->shape.getRadius(),
                                   eit->shape.getPosition(), eit->shape.getRadius())) {
                    eit->hp--;
                    hit = true;
                    if (eit->hp <= 0) {
                        eit = enemies.erase(eit); // Удаляем мёртвого врага
                    } else ++eit;
                    break; // Пуля может поразить только одного врага
                } else ++eit;
            }
            if (hit) it = bullets.erase(it);
            else ++it;
        }

        // 6. Рендеринг
        window.clear(sf::Color(20, 20, 30));
        window.draw(player);
        for (const auto& b : bullets) window.draw(b.shape);
        for (const auto& e : enemies) window.draw(e.shape);
        window.display();
    }

    return 0;
}
