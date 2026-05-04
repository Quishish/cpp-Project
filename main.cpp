#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include "Player.hpp"
#include "Enemy.hpp"
#include "ShooterEnemy.hpp"
#include "Bullet.hpp"
#include "Utils.hpp"

enum class GameState { Playing, GameOver };

int main() {
    const unsigned int WIDTH = 800;
    const unsigned int HEIGHT = 600;
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Bullet Hell - UI & Game Over");
    window.setFramerateLimit(60);

    // --- Состояние игры ---
    GameState state = GameState::Playing;
    const int MAX_HP = 3;

    // --- Игрок и пули ---
    Player player(15.f, 250.f, MAX_HP, sf::Vector2f(WIDTH / 2.f, HEIGHT / 2.f));
    std::vector<Enemy> enemies;
    std::vector<ShooterEnemy> shooters;
    std::vector<Bullet> pBullets;
    std::vector<Bullet> eBullets;

    // --- Таймеры ---
    sf::Clock deltaClock;
    sf::Clock spawnRegTimer;
    sf::Clock spawnShootTimer;
    const float SPAWN_REG_INTERVAL = 2.0f;
    const float SPAWN_SHOOT_INTERVAL = 8.0f;
    const float BULLET_SPEED = 400.f;
    const float SHOOT_COOLDOWN = 0.3f;
    float shootTimer = 0.f;

    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

    // --- UI: Полоска здоровья ---
    sf::RectangleShape hpBg(sf::Vector2f(200.f, 20.f));
    hpBg.setFillColor(sf::Color(40, 40, 40));
    hpBg.setPosition(20.f, 20.f);

    sf::RectangleShape hpFg(sf::Vector2f(200.f, 20.f));
    hpFg.setFillColor(sf::Color::Green);
    hpFg.setPosition(20.f, 20.f);

    // --- UI: Game Over экран ---
    sf::Font font;
    // Поместите любой .ttf файл в папку с игрой (например, arial.ttf)
    if (!font.loadFromFile("DejaVuSans.ttf")) {
        std::cerr << "Ошибка: не найден файл шрифта 'arial.ttf'. Скачайте и положите в папку с игрой.\n";
        return 1;
    }

    sf::Text goText("GAME OVER", font, 60);
    sf::FloatRect goBounds = goText.getLocalBounds();
    goText.setOrigin(goBounds.left + goBounds.width / 2.f, goBounds.top + goBounds.height / 2.f);
    goText.setPosition(WIDTH / 2.f, HEIGHT / 2.f - 40.f);
    goText.setFillColor(sf::Color::Red);

    sf::Text restartText("RESTART (Click or Press R)", font, 18);
    sf::FloatRect resBounds = restartText.getLocalBounds();
    restartText.setOrigin(resBounds.left + resBounds.width / 2.f, resBounds.top + resBounds.height / 2.f);
    restartText.setPosition(WIDTH / 2.f, HEIGHT / 2.f + 60.f);
    restartText.setFillColor(sf::Color::White);

    sf::RectangleShape restartBtn(sf::Vector2f(280.f, 50.f));
    restartBtn.setFillColor(sf::Color(50, 50, 50, 200));
    restartBtn.setOrigin(140.f, 25.f);
    restartBtn.setPosition(WIDTH / 2.f, HEIGHT / 2.f + 60.f);
    restartBtn.setOutlineThickness(2.f);
    restartBtn.setOutlineColor(sf::Color::White);

    sf::RectangleShape overlay(sf::Vector2f(WIDTH, HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (state == GameState::GameOver) {
                // Обработка рестарта
                bool clickedRestart = false;
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mPos = sf::Mouse::getPosition(window);
                    if (restartBtn.getGlobalBounds().contains(mPos.x, mPos.y)) clickedRestart = true;
                }
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::R) {
                    clickedRestart = true;
                }

                if (clickedRestart) {
                    state = GameState::Playing;
                    player = Player(15.f, 250.f, MAX_HP, sf::Vector2f(WIDTH / 2.f, HEIGHT / 2.f));
                    enemies.clear();
                    shooters.clear();
                    pBullets.clear();
                    eBullets.clear();
                    shootTimer = 0.f;
                    spawnRegTimer.restart();
                    spawnShootTimer.restart();
                }
            }
        }

        float dt = deltaClock.restart().asSeconds();
        shootTimer -= dt;

        // ==========================================
        // ЛОГИКА ИГРЫ (только в состоянии Playing)
        // ==========================================
        if (state == GameState::Playing) {
            // 1. Ввод WASD
            sf::Vector2f moveInput(0.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) moveInput.y -= 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) moveInput.y += 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) moveInput.x -= 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) moveInput.x += 1.f;
            if (moveInput.x != 0.f || moveInput.y != 0.f) moveInput /= std::hypot(moveInput.x, moveInput.y);
            player.setMovementInput(moveInput);
            player.update(dt);

            // Границы
            sf::Vector2f pPos = player.getPosition();
            float pr = player.getRadius();
            pPos.x = std::clamp(pPos.x, pr, static_cast<float>(WIDTH - pr));
            pPos.y = std::clamp(pPos.y, pr, static_cast<float>(HEIGHT - pr));
            player.setPosition(pPos);

            // 2. Стрельба игрока
            sf::Vector2f shootDir(0.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    shootDir.y = -1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  shootDir.y =  1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  shootDir.x = -1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) shootDir.x =  1.f;

            if ((shootDir.x != 0.f || shootDir.y != 0.f) && shootTimer <= 0.f) {
                shootDir /= std::hypot(shootDir.x, shootDir.y);
                Bullet b(5.f, sf::Color::Yellow);
                b.shape.setPosition(pPos + shootDir * (pr + 8.f));
                b.velocity = shootDir * BULLET_SPEED;
                pBullets.push_back(b);
                shootTimer = SHOOT_COOLDOWN;
            }

            // 3. Обновление пуль
            for (auto it = pBullets.begin(); it != pBullets.end(); ) {
                it->update(dt);
                if (it->isOffScreen(WIDTH, HEIGHT)) it = pBullets.erase(it); else ++it;
            }
            for (auto it = eBullets.begin(); it != eBullets.end(); ) {
                it->update(dt);
                if (it->isOffScreen(WIDTH, HEIGHT)) it = eBullets.erase(it); else ++it;
            }

            // 4. Враги
            pPos = player.getPosition();
            for (auto& e : enemies) e.chaseTarget(pPos, dt);
            for (auto& s : shooters) s.update(dt, pPos, eBullets);

            // 5. Разделение врагов (anti-clump)
            auto resolveClump = [&](auto& vec) {
                for (size_t i = 0; i < vec.size(); ++i)
                    for (size_t j = i + 1; j < vec.size(); ++j) {
                        sf::Vector2f diff = vec[i].getPosition() - vec[j].getPosition();
                        float dist = std::hypot(diff.x, diff.y);
                        float minD = vec[i].getRadius() + vec[j].getRadius();
                        if (dist < minD && dist > 0.001f) {
                            float ov = minD - dist;
                            sf::Vector2f dir = diff / dist;
                            vec[i].move(dir * ov * 0.5f);
                            vec[j].move(-dir * ov * 0.5f);
                        }
                    }
            };
            resolveClump(enemies);
            resolveClump(shooters);

            // 6. Отталкивание игрока
            for (const auto& e : enemies) if (checkCollision(player, e)) {
                sf::Vector2f push = pPos - e.getPosition(); float d = std::hypot(push.x, push.y);
                if (d > 0.f) player.move(push / d * 5.f);
            }
            for (const auto& s : shooters) if (checkCollision(player, s)) {
                sf::Vector2f push = pPos - s.getPosition(); float d = std::hypot(push.x, push.y);
                if (d > 0.f) player.move(push / d * 5.f);
            }

            // 7. Коллизии пуль игрока с врагами
            for (auto it = pBullets.begin(); it != pBullets.end(); ) {
                bool hit = false;
                for (auto eit = enemies.begin(); eit != enemies.end(); ) {
                    if (checkBulletCollision(it->shape, *eit)) {
                        eit->takeDamage(1); hit = true;
                        if (!eit->isAlive()) eit = enemies.erase(eit); else ++eit;
                        break;
                    } else ++eit;
                }
                if (hit) { it = pBullets.erase(it); continue; }
                for (auto sit = shooters.begin(); sit != shooters.end(); ) {
                    if (checkBulletCollision(it->shape, *sit)) {
                        sit->takeDamage(1); hit = true;
                        if (!sit->isAlive()) sit = shooters.erase(sit); else ++sit;
                        break;
                    } else ++sit;
                }
                if (hit) it = pBullets.erase(it); else ++it;
            }

            // 8. Коллизии вражеских пуль с игроком
            for (auto it = eBullets.begin(); it != eBullets.end(); ) {
                sf::Vector2f bPos = it->shape.getPosition();
                float dist = std::hypot(bPos.x - pPos.x, bPos.y - pPos.y);
                if (dist < it->shape.getRadius() + player.getRadius()) {
                    player.takeDamage(1);
                    it = eBullets.erase(it);
                } else ++it;
            }

            // 9. Спавн
            if (spawnRegTimer.getElapsedTime().asSeconds() >= SPAWN_REG_INTERVAL) {
                spawnRegTimer.restart();
                std::uniform_int_distribution<int> sideDist(0, 3);
                std::uniform_real_distribution<float> xDist(0.f, WIDTH), yDist(0.f, HEIGHT);
                float off = 30.f; sf::Vector2f sp;
                switch(sideDist(rng)) { case 0: sp={xDist(rng),-off}; break; case 1: sp={WIDTH+off,yDist(rng)}; break;
                    case 2: sp={xDist(rng),HEIGHT+off}; break; case 3: sp={-off,yDist(rng)}; break; }
                enemies.emplace_back(std::uniform_real_distribution<float>(15.f,25.f)(rng),
                                     std::uniform_real_distribution<float>(100.f,180.f)(rng),
                                     std::uniform_int_distribution<int>(1,3)(rng), sp);
            }
            if (spawnShootTimer.getElapsedTime().asSeconds() >= SPAWN_SHOOT_INTERVAL) {
                spawnShootTimer.restart();
                std::uniform_int_distribution<int> sideDist(0, 3);
                std::uniform_real_distribution<float> xDist(0.f, WIDTH), yDist(0.f, HEIGHT);
                float off = 40.f; sf::Vector2f sp;
                switch(sideDist(rng)) { case 0: sp={xDist(rng),-off}; break; case 1: sp={WIDTH+off,yDist(rng)}; break;
                    case 2: sp={xDist(rng),HEIGHT+off}; break; case 3: sp={-off,yDist(rng)}; break; }
                shooters.emplace_back(25.f, 80.f, 5, sp);
            }

            // 10. Проверка Game Over
            if (player.getHP() <= 0) state = GameState::GameOver;
        }

        // ==========================================
        // РЕНДЕРИНГ
        // ==========================================
        window.clear(sf::Color(20, 20, 30));

        // Игровые объекты
        for (const auto& b : pBullets) window.draw(b.shape);
        for (const auto& b : eBullets)   window.draw(b.shape);
        for (const auto& e : enemies)    e.draw(window);
        for (const auto& s : shooters)   s.draw(window);
        player.draw(window);

        if (state == GameState::Playing) {
            // HP Bar
            float ratio = static_cast<float>(player.getHP()) / MAX_HP;
            hpFg.setSize(sf::Vector2f(200.f * ratio, 20.f));
            if (ratio > 0.6f) hpFg.setFillColor(sf::Color::Green);
            else if (ratio > 0.3f) hpFg.setFillColor(sf::Color::Yellow);
            else hpFg.setFillColor(sf::Color::Red);
            window.draw(hpBg);
            window.draw(hpFg);
        } else {
            // Game Over UI
            window.draw(overlay);
            window.draw(restartBtn);
            window.draw(goText);
            window.draw(restartText);
        }

        window.display();
    }

    return 0;
}
