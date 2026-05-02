#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include "Player.hpp"
#include "Enemy.hpp"
#include "ShooterEnemy.hpp"
#include "Bullet.hpp"
#include "Utils.hpp"

int main() {
    const unsigned int WIDTH = 800;
    const unsigned int HEIGHT = 600;
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Bullet Hell - Full");
    window.setFramerateLimit(60);

    // --- Объекты ---
    Player player(15.f, 250.f, 3, sf::Vector2f(WIDTH / 2.f, HEIGHT / 2.f));
    std::vector<Enemy> enemies;
    std::vector<ShooterEnemy> shooters;
    std::vector<Bullet> pBullets; // Пули игрока
    std::vector<Bullet> eBullets; // Пули врагов

    // --- Таймеры и параметры ---
    const float BULLET_SPEED = 400.f;
    const float SHOOT_COOLDOWN = 0.3f;
    float shootTimer = 0.f;

    sf::Clock deltaClock;
    sf::Clock spawnRegTimer;
    sf::Clock spawnShootTimer;
    const float SPAWN_REG_INTERVAL = 2.0f;
    const float SPAWN_SHOOT_INTERVAL = 8.0f;

    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());


    while (window.isOpen()) {
        // 1. События
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        float dt = deltaClock.restart().asSeconds();
        shootTimer -= dt;

        // 2. Ввод игрока (WASD)
        sf::Vector2f moveInput(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W)) moveInput.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S)) moveInput.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A)) moveInput.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D)) moveInput.x += 1.f;

        if (moveInput.x != 0.f || moveInput.y != 0.f)
            moveInput /= std::hypot(moveInput.x, moveInput.y);

        player.setMovementInput(moveInput);
        player.update(dt);

        // Ограничение игрока границами
        float pr = player.getRadius();
        sf::Vector2f pPos = player.getPosition();
        pPos.x = std::clamp(pPos.x, pr, static_cast<float>(WIDTH - pr));
        pPos.y = std::clamp(pPos.y, pr, static_cast<float>(HEIGHT - pr));
        player.setPosition(pPos);

        // 3. Стрельба игрока (Стрелки)
        sf::Vector2f shootDir(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Up))    shootDir.y = -1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Down))  shootDir.y =  1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Left))  shootDir.x = -1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Right)) shootDir.x =  1.f;

        if ((shootDir.x != 0.f || shootDir.y != 0.f) && shootTimer <= 0.f) {
            shootDir /= std::hypot(shootDir.x, shootDir.y);
            Bullet b(5.f, sf::Color::Yellow);
            b.shape.setPosition(pPos + shootDir * (pr + 8.f));
            b.velocity = shootDir * BULLET_SPEED;
            pBullets.push_back(b);
            shootTimer = SHOOT_COOLDOWN;
        }

        // 4. Обновление пуль
        for (auto it = pBullets.begin(); it != pBullets.end(); ) {
            it->update(dt);
            if (it->isOffScreen(WIDTH, HEIGHT)) it = pBullets.erase(it);
            else ++it;
        }

        for (auto it = eBullets.begin(); it != eBullets.end(); ) {
            it->update(dt);
            if (it->isOffScreen(WIDTH, HEIGHT)) it = eBullets.erase(it);
            else ++it;
        }

        // 5. Обновление врагов
        pPos = player.getPosition();
        for (auto& e : enemies) e.chaseTarget(pPos, dt);
        for (auto& s : shooters) s.update(dt, pPos, eBullets);

        // 6. Разрешение коллизий Враг ↔ Враг (чтобы не слипались)
        for (size_t i = 0; i < enemies.size(); ++i) {
            for (size_t j = i + 1; j < enemies.size(); ++j) {
                sf::Vector2f diff = enemies[i].getPosition() - enemies[j].getPosition();
                float dist = std::hypot(diff.x, diff.y);
                float minDist = enemies[i].getRadius() + enemies[j].getRadius();
                if (dist < minDist && dist > 0.001f) {
                    float overlap = minDist - dist;
                    sf::Vector2f dir = diff / dist;
                    enemies[i].move(dir * overlap * 0.5f);
                    enemies[j].move(-dir * overlap * 0.5f);
                }
            }
        }

        for (size_t i = 0; i < shooters.size(); ++i) {
            for (size_t j = i + 1; j < shooters.size(); ++j) {
                sf::Vector2f diff = shooters[i].getPosition() - shooters[j].getPosition();
                float dist = std::hypot(diff.x, diff.y);
                float minDist = shooters[i].getRadius() + shooters[j].getRadius();
                if (dist < minDist && dist > 0.001f) {
                    float overlap = minDist - dist;
                    sf::Vector2f dir = diff / dist;
                    shooters[i].move(dir * overlap * 0.5f);
                    shooters[j].move(-dir * overlap * 0.5f);
                }
            }
        }

        // 7. Отталкивание игрока при касании врагов
        for (const auto& e : enemies) {
            if (checkCollision(player, e)) {
                sf::Vector2f push = pPos - e.getPosition();
                float d = std::hypot(push.x, push.y);
                if (d > 0.f) player.move(push / d * 5.f);
            }
        }
        for (const auto& s : shooters) {
            if (checkCollision(player, s)) {
                sf::Vector2f push = pPos - s.getPosition();
                float d = std::hypot(push.x, push.y);
                if (d > 0.f) player.move(push / d * 5.f);
            }
        }

        // 8. Коллизии: Пули Игрока ↔ Враги
        for (auto it = pBullets.begin(); it != pBullets.end(); ) {
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
            if (hit) { it = pBullets.erase(it); continue; }

            for (auto sit = shooters.begin(); sit != shooters.end(); ) {
                if (checkBulletCollision(it->shape, *sit)) {
                    sit->takeDamage(1);
                    hit = true;
                    if (!sit->isAlive()) sit = shooters.erase(sit);
                    else ++sit;
                    break;
                } else ++sit;
            }
            if (hit) it = pBullets.erase(it);
            else ++it;
        }

        // 9. Коллизии: Пули Врагов ↔ Игрок
        for (auto it = eBullets.begin(); it != eBullets.end(); ) {
            sf::Vector2f bPos = it->shape.getPosition();
            float dist = std::hypot(bPos.x - pPos.x, bPos.y - pPos.y);
            if (dist < it->shape.getRadius() + player.getRadius()) {
                player.takeDamage(1);
                it = eBullets.erase(it);
            } else ++it;
        }

        // 10. Спавн обычных врагов
        if (spawnRegTimer.getElapsedTime().asSeconds() >= SPAWN_REG_INTERVAL) {
            spawnRegTimer.restart();
            std::uniform_int_distribution<int> sideDist(0, 3);
            std::uniform_real_distribution<float> xDist(0.f, static_cast<float>(WIDTH));
            std::uniform_real_distribution<float> yDist(0.f, static_cast<float>(HEIGHT));
            std::uniform_real_distribution<float> radDist(15.f, 25.f);
            std::uniform_real_distribution<float> spdDist(100.f, 180.f);
            std::uniform_int_distribution<int> hpDist(1, 3);

            float off = 30.f;
            sf::Vector2f sp;
            int side = sideDist(rng);
            switch(side) {
                case 0: sp = {xDist(rng), -off}; break;
                case 1: sp = {WIDTH + off, yDist(rng)}; break;
                case 2: sp = {xDist(rng), HEIGHT + off}; break;
                case 3: sp = {-off, yDist(rng)}; break;
            }
            enemies.emplace_back(radDist(rng), spdDist(rng), hpDist(rng), sp);
        }

        // 11. Спавн стреляющих врагов
        if (spawnShootTimer.getElapsedTime().asSeconds() >= SPAWN_SHOOT_INTERVAL) {
            spawnShootTimer.restart();
            std::uniform_int_distribution<int> sideDist(0, 3);
            std::uniform_real_distribution<float> xDist(0.f, static_cast<float>(WIDTH));
            std::uniform_real_distribution<float> yDist(0.f, static_cast<float>(HEIGHT));
            float off = 40.f;
            sf::Vector2f sp;
            int side = sideDist(rng);
            switch(side) {
                case 0: sp = {xDist(rng), -off}; break;
                case 1: sp = {WIDTH + off, yDist(rng)}; break;
                case 2: sp = {xDist(rng), HEIGHT + off}; break;
                case 3: sp = {-off, yDist(rng)}; break;
            }
            shooters.emplace_back(25.f, 80.f, 5, sp);
        }

        // 12. Рендеринг
        window.clear(sf::Color(20, 20, 30));
        player.draw(window);
        for (const auto& b : pBullets) window.draw(b.shape);
        for (const auto& b : eBullets)   window.draw(b.shape);
        for (const auto& e : enemies)    e.draw(window);
        for (const auto& s : shooters)   s.draw(window);
        window.display();
    }

    return 0;
}
