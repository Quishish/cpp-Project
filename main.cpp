#include <SFML/Graphics.hpp>
#include <algorithm>
#include <vector>
#include <random>      // <-- НОВОЕ: Современный генератор случайных чисел
#include <chrono>      // <-- НОВОЕ: Для seed таймера
#include "Player.hpp"
#include "Enemy.hpp"
#include "Bullet.hpp"
#include "Utils.hpp"

int main() {
    const unsigned int WIDTH = 800;
    const unsigned int HEIGHT = 600;
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Bullet Hell - Spawner");
    window.setFramerateLimit(60);

    Player player(15.f, 250.f, 3, sf::Vector2f(WIDTH / 2.f, HEIGHT / 2.f));
    std::vector<Enemy> enemies;
    enemies.emplace_back(20.f, 120.f, 3, sf::Vector2f(100.f, 100.f));
    enemies.emplace_back(20.f, 140.f, 2, sf::Vector2f(600.f, 400.f));

    std::vector<Bullet> bullets;

    const float BULLET_SPEED = 400.f;
    const float SHOOT_COOLDOWN = 0.3f;
    float shootTimer = 0.f;
    sf::Clock deltaClock;

    // --- НОВОЕ: Параметры спавна ---
    sf::Clock spawnTimer;
    const float SPAWN_INTERVAL = 2.0f; // Спавн каждые 2 секунды
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count()); // Генератор

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        float dt = deltaClock.restart().asSeconds();
        shootTimer -= dt;

        // --- ВВОД (WASD) ---
        sf::Vector2f moveInput(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W)) moveInput.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S)) moveInput.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A)) moveInput.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D)) moveInput.x += 1.f;

        if (moveInput.x != 0.f || moveInput.y != 0.f)
            moveInput /= std::hypot(moveInput.x, moveInput.y);

        player.setMovementInput(moveInput);
        player.update(dt);

        float pr = player.getRadius();
        sf::Vector2f pos = player.getPosition();
        pos.x = std::clamp(pos.x, pr, static_cast<float>(WIDTH - pr));
        pos.y = std::clamp(pos.y, pr, static_cast<float>(HEIGHT - pr));
        player.setPosition(pos);

        // --- СТРЕЛЬБА ---
        sf::Vector2f shootDir(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Up))    shootDir.y = -1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Down))  shootDir.y =  1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Left))  shootDir.x = -1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Right)) shootDir.x =  1.f;

        if ((shootDir.x != 0.f || shootDir.y != 0.f) && shootTimer <= 0.f) {
            shootDir /= std::hypot(shootDir.x, shootDir.y);
            Bullet b(5.f, sf::Color::Yellow);
            b.shape.setPosition(player.getPosition() + shootDir * (pr + 8.f));
            b.velocity = shootDir * BULLET_SPEED;
            bullets.push_back(b);
            shootTimer = SHOOT_COOLDOWN;
        }

        // --- ОБНОВЛЕНИЕ ПУЛЬ ---
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            it->update(dt);
            if (it->isOffScreen(WIDTH, HEIGHT)) it = bullets.erase(it);
            else ++it;
        }

        // --- ОБНОВЛЕНИЕ ВРАГОВ ---
        sf::Vector2f pPos = player.getPosition();
        for (auto& enemy : enemies) {
            enemy.chaseTarget(pPos, dt);
            if (checkCollision(enemy, player)) {
                sf::Vector2f pushDir = pPos - enemy.getPosition();
                float dist = std::hypot(pushDir.x, pushDir.y);
                if (dist > 0.f) { pushDir /= dist; player.move(pushDir * 5.f); }
            }
        }

        // --- СТОЛКНОВЕНИЕ ПУЛИ ↔ ВРАГИ ---
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            bool hit = false;
            for (auto eit = enemies.begin(); eit != enemies.end(); ) {
                if (checkBulletCollision(it->shape, *eit)) {
                    eit->takeDamage(1);
                    hit = true;
                    if (!eit->isAlive()) eit = enemies.erase(eit);
                    else ++eit;
                    break;
                } else ++eit;
            }
            if (hit) it = bullets.erase(it);
            else ++it;
        }

        // ==========================================
        // --- НОВОЕ: ЛОГИКА СПАВНА ВРАГОВ ---
        // ==========================================
        if (spawnTimer.getElapsedTime().asSeconds() >= SPAWN_INTERVAL) {
            spawnTimer.restart();

            // 1. Выбираем случайную сторону появления (0:верх, 1:право, 2:низ, 3:лево)
            std::uniform_int_distribution<int> sideDist(0, 3);
            int side = sideDist(rng);

            // 2. Генерируем координаты за пределами экрана
            std::uniform_real_distribution<float> xDist(0.f, static_cast<float>(WIDTH));
            std::uniform_real_distribution<float> yDist(0.f, static_cast<float>(HEIGHT));

            sf::Vector2f spawnPos;
            float offset = 30.f; // Насколько далеко за экраном спавнить
            switch (side) {
                case 0: spawnPos = {xDist(rng), -offset}; break;
                case 1: spawnPos = {WIDTH + offset, yDist(rng)}; break;
                case 2: spawnPos = {xDist(rng), HEIGHT + offset}; break;
                case 3: spawnPos = {-offset, yDist(rng)}; break;
            }

            // 3. Случайные параметры врага
            std::uniform_real_distribution<float> radDist(15.f, 25.f);
            std::uniform_real_distribution<float> spdDist(100.f, 180.f);
            std::uniform_int_distribution<int> hpDist(1, 3);

            enemies.emplace_back(radDist(rng), spdDist(rng), hpDist(rng), spawnPos);
        }

        // --- РЕНДЕРИНГ ---
        window.clear(sf::Color(20, 20, 30));
        player.draw(window);
        for (const auto& b : bullets) window.draw(b.shape);
        for (const auto& e : enemies) e.draw(window);
        window.display();
    }

    return 0;
}
