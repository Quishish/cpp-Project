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
#include "powerUp.hpp"

enum class GameState { MainMenu, Playing, GameOver };

int main() {
    constexpr float GAME_WIDTH  = 1600.f;
    constexpr float GAME_HEIGHT = 1000.f;
    const sf::Vector2f GAME_SIZE(GAME_WIDTH, GAME_HEIGHT);

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Bullet Hell - Fullscreen 16:9", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    sf::View gameView;
    gameView.setSize(GAME_SIZE);
    gameView.setCenter(GAME_SIZE / 2.f);

    auto updateViewport = [&]() {
        sf::Vector2u winSize = window.getSize();
        float winAspect = static_cast<float>(winSize.x) / winSize.y;
        float gameAspect = GAME_WIDTH / GAME_HEIGHT;
        float vpWidth = 1.f, vpHeight = 1.f;
        if (winAspect > gameAspect) vpWidth = gameAspect / winAspect;
        else vpHeight = winAspect / gameAspect;
        gameView.setViewport(sf::FloatRect((1.f - vpWidth) / 2.f, (1.f - vpHeight) / 2.f, vpWidth, vpHeight));
    };
    updateViewport();
    window.setView(gameView);

    GameState state = GameState::MainMenu;  // ← Начинаем с меню, не с игры!
    int selectedButton = 0;  // для Game Over
    int menuSelected = 0;    // ← для MainMenu: 0=Start, 1=Exit
    const int MAX_HP = 3;

    Player player({30.f, 30.f}, 250.f, MAX_HP, GAME_SIZE / 2.f);
    std::vector<Enemy> enemies;
    std::vector<ShooterEnemy> shooters;
    std::vector<Bullet> pBullets;
    std::vector<Bullet> eBullets;
    std::vector<PowerUp> powerups;

    sf::Clock deltaClock;
    sf::Clock spawnRegTimer;
    sf::Clock spawnShootTimer;
    const float SPAWN_REG_INTERVAL = 2.0f;
    const float SPAWN_SHOOT_INTERVAL = 8.0f;
    const float BULLET_SPEED = 400.f;
    const float SHOOT_COOLDOWN = 0.3f;
    float shootTimer = 0.f;

    //переменные для павер-апов
    constexpr int MAX_POWERUPS = 4;                    // было 2
    constexpr float SPAWN_CHANCE_PER_FRAME = 0.008f;   // было 0.002 (×4 чаще)
    constexpr float MIN_SPAWN_DISTANCE = 150.0f;
    constexpr float SHIELD_DURATION = 5.0f;
    bool isInvulnerable = false;
    sf::Clock shieldTimer;

    bool hasSpeedBoost = false;
    bool hasRapidFire = false;
    sf::Clock speedTimer;
    sf::Clock rapidFireTimer;
    constexpr float BUFF_DURATION = 5.0f;
    constexpr float BUFF_MULTIPLIER = 1.5f;
    const float BASE_PLAYER_SPEED = 250.f;      // сохраните базовую скорость
    const float BASE_SHOOT_COOLDOWN = 0.3f;     // сохраните базовый кулдаун

    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::srand(std::chrono::steady_clock::now().time_since_epoch().count());

    // --- UI: HP Bar ---
    sf::RectangleShape hpBg(sf::Vector2f(200.f, 20.f));
    hpBg.setFillColor(sf::Color(40, 40, 40));
    hpBg.setPosition(20.f, 20.f);
    sf::RectangleShape hpFg(sf::Vector2f(200.f, 20.f));
    hpFg.setFillColor(sf::Color::Green);
    hpFg.setPosition(20.f, 20.f);

    sf::RectangleShape buffBarBg(sf::Vector2f(200.f, 12.f));
    buffBarBg.setFillColor(sf::Color(40, 40, 40, 180));
    buffBarBg.setPosition(20.f, 45.f);  // чуть ниже HP-бара

    // Щит: синий
    sf::RectangleShape shieldBar(sf::Vector2f(200.f, 12.f));
    shieldBar.setFillColor(sf::Color(30, 144, 255));
    shieldBar.setPosition(20.f, 45.f);

    // Скорость: зелёный
    sf::RectangleShape speedBar(sf::Vector2f(200.f, 12.f));
    speedBar.setFillColor(sf::Color(50, 205, 50));
    speedBar.setPosition(20.f, 60.f);  // ещё ниже

    // Скорострельность: розовый
    sf::RectangleShape rapidBar(sf::Vector2f(200.f, 12.f));
    rapidBar.setFillColor(sf::Color(255, 105, 180));
    rapidBar.setPosition(20.f, 75.f);

    // --- UI: Шрифт и тексты ---
    sf::Font font;
    if (!font.loadFromFile("resources/fonts/DejaVuSans.ttf")) {
        std::cerr << "Ошибка: не найден файл шрифта 'resources/fonts/DejaVuSans.ttf'\n";
        return 1;
    }

        // Текстовые подписи (опционально)
    sf::Text shieldLabel("SHIELD", font, 10);
    shieldLabel.setPosition(25.f, 47.f);
    shieldLabel.setFillColor(sf::Color::White);

    sf::Text speedLabel("SPEED", font, 10);
    speedLabel.setPosition(25.f, 62.f);
    speedLabel.setFillColor(sf::Color::White);

    sf::Text rapidLabel("RAPID", font, 10);
    rapidLabel.setPosition(25.f, 77.f);

    sf::Text goText("GAME OVER", font, 60);
    sf::FloatRect goBounds = goText.getLocalBounds();
    goText.setOrigin(goBounds.left + goBounds.width / 2.f, goBounds.top + goBounds.height / 2.f);
    goText.setPosition(GAME_WIDTH / 2.f, GAME_HEIGHT / 2.f - 50.f);
    goText.setFillColor(sf::Color::Red);

    sf::RectangleShape restartBtn(sf::Vector2f(280.f, 50.f));
    restartBtn.setOrigin(140.f, 25.f);
    restartBtn.setPosition(GAME_WIDTH / 2.f - 160.f, GAME_HEIGHT / 2.f + 40.f);

    sf::Text restartText("RESTART (R)", font, 18);
    sf::FloatRect resBounds = restartText.getLocalBounds();
    restartText.setOrigin(resBounds.left + resBounds.width / 2.f, resBounds.top + resBounds.height / 2.f);
    restartText.setPosition(restartBtn.getPosition());
    restartText.setFillColor(sf::Color::White);

    sf::RectangleShape exitBtn(sf::Vector2f(280.f, 50.f));
    exitBtn.setOrigin(140.f, 25.f);
    exitBtn.setPosition(GAME_WIDTH / 2.f + 160.f, GAME_HEIGHT / 2.f + 40.f);

    sf::Text exitText("EXIT (Esc)", font, 18);
    sf::FloatRect exitBounds = exitText.getLocalBounds();
    exitText.setOrigin(exitBounds.left + exitBounds.width / 2.f, exitBounds.top + exitBounds.height / 2.f);
    exitText.setPosition(exitBtn.getPosition());
    exitText.setFillColor(sf::Color::White);

    // ============================================================================
    // === UI: ГЛАВНОЕ МЕНЮ (объявления переменных) ===
    // ============================================================================

    // Заголовок меню
    sf::Text menuTitle("BULLET HELL", font, 72);
    sf::FloatRect titleBounds = menuTitle.getLocalBounds();
    menuTitle.setOrigin(titleBounds.left + titleBounds.width / 2.f, titleBounds.top + titleBounds.height / 2.f);
    menuTitle.setPosition(GAME_WIDTH / 2.f, GAME_HEIGHT / 2.f - 100.f);
    menuTitle.setFillColor(sf::Color::Cyan);

    // Кнопка START (верхняя)
    sf::RectangleShape startBtn(sf::Vector2f(280.f, 50.f));
    startBtn.setOrigin(140.f, 25.f);
    startBtn.setPosition(GAME_WIDTH / 2.f, GAME_HEIGHT / 2.f + 20.f);  // по центру, выше
    startBtn.setOutlineThickness(2.f);
    startBtn.setOutlineColor(sf::Color::White);
    startBtn.setFillColor(sf::Color(50, 50, 50, 200));

    sf::Text startText("START GAME", font, 18);
    sf::FloatRect startBounds = startText.getLocalBounds();
    startText.setOrigin(startBounds.left + startBounds.width / 2.f, startBounds.top + startBounds.height / 2.f);
    startText.setPosition(startBtn.getPosition());
    startText.setFillColor(sf::Color::White);

    // Кнопка EXIT (нижняя, под START)
    sf::RectangleShape menuExitBtn(sf::Vector2f(280.f, 50.f));  // ← имя menuExitBtn, чтобы не конфликтовать с exitBtn из Game Over
    menuExitBtn.setOrigin(140.f, 25.f);
    menuExitBtn.setPosition(GAME_WIDTH / 2.f, GAME_HEIGHT / 2.f + 90.f);  // на 70px ниже startBtn
    menuExitBtn.setOutlineThickness(2.f);
    menuExitBtn.setOutlineColor(sf::Color::White);
    menuExitBtn.setFillColor(sf::Color(50, 50, 50, 200));

    sf::Text menuExitText("EXIT GAME", font, 18);
    sf::FloatRect menuExitBounds = menuExitText.getLocalBounds();
    menuExitText.setOrigin(menuExitBounds.left + menuExitBounds.width / 2.f, menuExitBounds.top + menuExitBounds.height / 2.f);
    menuExitText.setPosition(menuExitBtn.getPosition());
    menuExitText.setFillColor(sf::Color::White);

    // Оверлей для меню (такой же как в Game Over)
    sf::RectangleShape menuOverlay(sf::Vector2f(GAME_WIDTH, GAME_HEIGHT));
    menuOverlay.setFillColor(sf::Color(0, 0, 0, 180));

    // ============================================================================

    sf::RectangleShape overlay(sf::Vector2f(GAME_WIDTH, GAME_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));

    // Лямбда для быстрого обновления визуала кнопок
    auto updateButtonVisuals = [](sf::RectangleShape& btn, sf::Text& txt, bool selected) {
        btn.setOutlineColor(selected ? sf::Color::Yellow : sf::Color::White);
        btn.setOutlineThickness(selected ? 3.f : 2.f);
        btn.setFillColor(selected ? sf::Color(70, 70, 70, 200) : sf::Color(50, 50, 50, 200));
        txt.setFillColor(selected ? sf::Color::Yellow : sf::Color::White);
    };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) {
                updateViewport();
                window.setView(gameView);
            }

            // === ОБРАБОТКА: ГЛАВНОЕ МЕНЮ ===
            if (state == GameState::MainMenu) {
                // Клавиатурная навигация
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Left) {
                        menuSelected = 0;  // START
                    }
                    if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Right) {
                        menuSelected = 1;  // EXIT
                    }
                    // Активация: Enter или пробел
                    if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                        if (menuSelected == 0) {
                            state = GameState::Playing;  // Начать игру
                        } else {
                            window.close();  // Выйти
                        }
                    }
                }
                
                // Мышь: наведение + клик
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mPos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldPos = window.mapPixelToCoords(mPos, window.getView());
                    
                    if (startBtn.getGlobalBounds().contains(worldPos)) {
                        menuSelected = 0;
                        state = GameState::Playing;
                    }
                    if (menuExitBtn.getGlobalBounds().contains(worldPos)) {
                        menuSelected = 1;
                        window.close();
                    }
                }
            }

            if (state == GameState::GameOver) {
                bool actionTriggered = false;

                // Клавиатурная навигация
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Left)  selectedButton = 0;
                    if (event.key.code == sf::Keyboard::Right) selectedButton = 1;
                    if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return) actionTriggered = true;
                    if (event.key.code == sf::Keyboard::R) { selectedButton = 0; actionTriggered = true; }
                    if (event.key.code == sf::Keyboard::Escape) { selectedButton = 1; actionTriggered = true; }
                }

                // Мышь: наведение меняет фокус, клик активирует
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mPos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldPos = window.mapPixelToCoords(mPos, window.getView());
                    if (restartBtn.getGlobalBounds().contains(worldPos)) selectedButton = 0;
                    if (exitBtn.getGlobalBounds().contains(worldPos)) selectedButton = 1;
                    actionTriggered = true;
                }

                // Выполнение действия
                if (actionTriggered) {
                    if (selectedButton == 0) {
                        state = GameState::Playing;
                        player = Player({30.f, 30.f}, 250.f, MAX_HP, GAME_SIZE / 2.f);
                        enemies.clear(); shooters.clear(); pBullets.clear(); eBullets.clear(); powerups.clear();
                        shootTimer = 0.f;
                        spawnRegTimer.restart(); spawnShootTimer.restart();
                        selectedButton = 0; // Сброс выбора
                        isInvulnerable = false;
                        hasSpeedBoost = false;
                        hasRapidFire = false;
                        shieldTimer.restart();
                        speedTimer.restart();
                        rapidFireTimer.restart();
                        player.setSpeed(BASE_PLAYER_SPEED);
                    } else {
                        state = GameState::MainMenu;  // ← вернуться в меню
                        menuSelected = 0;             // сброс выбора
                    }
                }
            }
        }

        float dt = deltaClock.restart().asSeconds();
        shootTimer -= dt;

        // Проверка истечения баффов
        if (hasSpeedBoost && speedTimer.getElapsedTime().asSeconds() >= BUFF_DURATION) {
            hasSpeedBoost = false;
            player.setSpeed(BASE_PLAYER_SPEED);  // восстанавливаем базовую скорость
        }
        if (hasRapidFire && rapidFireTimer.getElapsedTime().asSeconds() >= BUFF_DURATION) {
            hasRapidFire = false;
        }

        if (isInvulnerable && shieldTimer.getElapsedTime().asSeconds() >= SHIELD_DURATION) {
            isInvulnerable = false;
        }

        if (state == GameState::Playing) {
            sf::Vector2f moveInput(0.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) moveInput.y -= 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) moveInput.y += 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) moveInput.x -= 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) moveInput.x += 1.f;
            if (moveInput.x != 0.f || moveInput.y != 0.f) moveInput /= std::hypot(moveInput.x, moveInput.y);
            player.setMovementInput(moveInput);
            player.update(dt);

            sf::Vector2f pPos = player.getPosition();
            sf::Vector2f hs = player.getHalfSize();
            pPos.x = std::clamp(pPos.x, hs.x, GAME_WIDTH - hs.x);
            pPos.y = std::clamp(pPos.y, hs.y, GAME_HEIGHT - hs.y);
            player.setPosition(pPos);

            sf::Vector2f shootDir(0.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    shootDir.y = -1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  shootDir.y =  1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  shootDir.x = -1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) shootDir.x =  1.f;

            float currentCooldown = hasRapidFire ? BASE_SHOOT_COOLDOWN / BUFF_MULTIPLIER : BASE_SHOOT_COOLDOWN;
            if ((shootDir.x != 0.f || shootDir.y != 0.f) && shootTimer <= 0.f) {
                shootDir /= std::hypot(shootDir.x, shootDir.y);
                Bullet b({10.f, 10.f}, sf::Color::Yellow);
                b.shape.setPosition(pPos + shootDir * (hs.x + 8.f));
                b.velocity = shootDir * BULLET_SPEED;
                pBullets.push_back(b);
                shootTimer = currentCooldown;  // используем модифицированный кулдаун
            }

            for (auto it = pBullets.begin(); it != pBullets.end(); ) {
                it->update(dt);
                if (it->isOffScreen(GAME_WIDTH, GAME_HEIGHT)) it = pBullets.erase(it); else ++it;
            }
            for (auto it = eBullets.begin(); it != eBullets.end(); ) {
                it->update(dt);
                if (it->isOffScreen(GAME_WIDTH, GAME_HEIGHT)) it = eBullets.erase(it); else ++it;
            }

            pPos = player.getPosition();
            for (auto& e : enemies) e.chaseTarget(pPos, dt);
            for (auto& s : shooters) s.update(dt, pPos, eBullets);

            auto resolveClump = [&](auto& vec) {
                for (size_t i = 0; i < vec.size(); ++i)
                    for (size_t j = i + 1; j < vec.size(); ++j) {
                        sf::Vector2f diff = vec[i].getPosition() - vec[j].getPosition();
                        float dist = std::hypot(diff.x, diff.y);
                        float minD = std::max(vec[i].getHalfSize().x + vec[j].getHalfSize().x,
                                              vec[i].getHalfSize().y + vec[j].getHalfSize().y);
                        if (dist < minD && dist > 0.001f) {
                            float ov = minD - dist;
                            sf::Vector2f dir = diff / dist;
                            vec[i].move(dir * ov * 0.5f);
                            vec[j].move(-dir * ov * 0.5f);
                        }
                    }
            };
            resolveClump(enemies); resolveClump(shooters);

            for (auto& p : powerups) p.update(dt);

            const int CONTACT_DAMAGE = 1;
            const float CONTACT_COOLDOWN = 0.5f;
            static sf::Clock contactTimer;
            auto applyContact = [&](Entity& enemy) {
                if (checkCollision(player, enemy)) {
                    // Отталкивание — работает ВСЕГДА, даже со щитом
                    sf::Vector2f push = pPos - enemy.getPosition();
                    float d = std::hypot(push.x, push.y);
                    if (d > 0.f) player.move(push / d * 5.f);

                    // Урон — только если нет щита
                    if (!isInvulnerable) {
                        if (contactTimer.getElapsedTime().asSeconds() >= CONTACT_COOLDOWN) {
                            player.takeDamage(CONTACT_DAMAGE);
                            contactTimer.restart();
                        }
                    }
                }
            };
            for (auto& e : enemies) applyContact(e);
            for (auto& s : shooters) applyContact(s);

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

            for (auto it = eBullets.begin(); it != eBullets.end(); ) {
                if (!isInvulnerable && it->shape.getGlobalBounds().intersects(player.getGlobalBounds())) {
                    player.takeDamage(1);
                    it = eBullets.erase(it);
                } else ++it;
            }

            if (isInvulnerable && shieldTimer.getElapsedTime().asSeconds() >= SHIELD_DURATION) {
                isInvulnerable = false;
            }
            if (hasSpeedBoost && speedTimer.getElapsedTime().asSeconds() >= BUFF_DURATION) {
                hasSpeedBoost = false;
                player.setSpeed(BASE_PLAYER_SPEED);
            }
            if (hasRapidFire && rapidFireTimer.getElapsedTime().asSeconds() >= BUFF_DURATION) {
                hasRapidFire = false;
            }

            // === UPDATE BUFF BARS ===
            constexpr float BAR_MAX_WIDTH = 200.f;

            // Щит
            if (isInvulnerable) {
                float elapsed = shieldTimer.getElapsedTime().asSeconds();
                float ratio = 1.f - (elapsed / SHIELD_DURATION);
                shieldBar.setSize(sf::Vector2f(BAR_MAX_WIDTH * std::max(0.f, ratio), 12.f));
            } else {
                shieldBar.setSize(sf::Vector2f(0.f, 12.f));  // скрыть
            }

            // Скорость
            if (hasSpeedBoost) {
                float elapsed = speedTimer.getElapsedTime().asSeconds();
                float ratio = 1.f - (elapsed / BUFF_DURATION);
                speedBar.setSize(sf::Vector2f(BAR_MAX_WIDTH * std::max(0.f, ratio), 12.f));
            } else {
                speedBar.setSize(sf::Vector2f(0.f, 12.f));
            }

            // Скорострельность
            if (hasRapidFire) {
                float elapsed = rapidFireTimer.getElapsedTime().asSeconds();
                float ratio = 1.f - (elapsed / BUFF_DURATION);
                rapidBar.setSize(sf::Vector2f(BAR_MAX_WIDTH * std::max(0.f, ratio), 12.f));
            } else {
                rapidBar.setSize(sf::Vector2f(0.f, 12.f));
            }

            // === COLLECT POWER-UPS ===
            for (auto it = powerups.begin(); it != powerups.end(); ) {
                if (checkCollision(player, *it)) {
                    switch (it->getType()) {
                        case PowerUp::Type::Medkit: {
                            int heal = it->getHealAmount();
                            int currentHp = player.getHP();
                            int newHp = std::min(currentHp + heal, MAX_HP);
                            if (newHp > currentHp) {
                                player.takeDamage(-(newHp - currentHp));  // лечение через отрицательный урон
                            }
                            break;
                        }
                        case PowerUp::Type::Shield:
                            isInvulnerable = true;      // активируем (или оставляем активным)
                            shieldTimer.restart();      // ← КЛЮЧЕВОЕ: перезапускаем таймер
                            break;
                        case PowerUp::Type::Speed:
                            hasSpeedBoost = true;       // активируем (или оставляем активным)
                            player.setSpeed(BASE_PLAYER_SPEED * BUFF_MULTIPLIER);  // применяем скорость
                            speedTimer.restart();       // ← КЛЮЧЕВОЕ: перезапускаем таймер
                            break;
                        case PowerUp::Type::RapidFire:
                            hasRapidFire = true;        // активируем (или оставляем активным)
                            rapidFireTimer.restart();   // ← КЛЮЧЕВОЕ: перезапускаем таймер
                            break;
                    }
                    it = powerups.erase(it);
                } else {
                    ++it;
                }
            }
            if (spawnRegTimer.getElapsedTime().asSeconds() >= SPAWN_REG_INTERVAL) {
                spawnRegTimer.restart();
                std::uniform_int_distribution<int> sideDist(0, 3);
                std::uniform_real_distribution<float> xDist(0.f, GAME_WIDTH), yDist(0.f, GAME_HEIGHT);
                float off = 60.f; sf::Vector2f sp;
                switch(sideDist(rng)) { case 0: sp={xDist(rng),-off}; break; case 1: sp={GAME_WIDTH+off,yDist(rng)}; break;
                    case 2: sp={xDist(rng),GAME_HEIGHT+off}; break; case 3: sp={-off,yDist(rng)}; break; }
                float randSize = std::uniform_real_distribution<float>(30.f, 50.f)(rng);
                enemies.emplace_back(sf::Vector2f(randSize, randSize),
                                     std::uniform_real_distribution<float>(100.f, 180.f)(rng),
                                     std::uniform_int_distribution<int>(1, 3)(rng), sp);
            }
            if (spawnShootTimer.getElapsedTime().asSeconds() >= SPAWN_SHOOT_INTERVAL) {
                spawnShootTimer.restart();
                std::uniform_int_distribution<int> sideDist(0, 3);
                std::uniform_real_distribution<float> xDist(0.f, GAME_WIDTH), yDist(0.f, GAME_HEIGHT);
                float off = 70.f; sf::Vector2f sp;
                switch(sideDist(rng)) { case 0: sp={xDist(rng),-off}; break; case 1: sp={GAME_WIDTH+off,yDist(rng)}; break;
                    case 2: sp={xDist(rng),GAME_HEIGHT+off}; break; case 3: sp={-off,yDist(rng)}; break; }
                shooters.emplace_back(sf::Vector2f(50.f, 50.f), 80.f, 5, sp);
            }

            if (powerups.size() < MAX_POWERUPS) {
                std::uniform_real_distribution<float> chanceDist(0.f, 1.f);
                if (chanceDist(rng) < SPAWN_CHANCE_PER_FRAME) {
                    sf::Vector2f spawnPos = PowerUp::generateRandomPosition(
                        window.getSize(), player.getGlobalBounds(), MIN_SPAWN_DISTANCE
                    );
                    std::uniform_int_distribution<int> typeDist(0, 3);  // 0=Medkit, 1=Shield, 2=Speed, 3=RapidFire
                    PowerUp::Type type = static_cast<PowerUp::Type>(typeDist(rng));
                    powerups.emplace_back(spawnPos.x, spawnPos.y, type);
                }
            }




            if (player.getHP() <= 0) state = GameState::GameOver;
        }

        // --- РЕНДЕРИНГ ---
        window.clear(sf::Color(20, 20, 30));
        for (const auto& b : pBullets) window.draw(b.shape);
        for (const auto& b : eBullets)   window.draw(b.shape);
        for (const auto& e : enemies)    e.draw(window);
        for (const auto& s : shooters)   s.draw(window);
        for (const auto& p : powerups) p.draw(window);
        player.draw(window);

        if (state == GameState::Playing) {
            float ratio = static_cast<float>(player.getHP()) / MAX_HP;
            hpFg.setSize(sf::Vector2f(200.f * ratio, 20.f));
            if (ratio > 0.6f) hpFg.setFillColor(sf::Color::Green);
            else if (ratio > 0.3f) hpFg.setFillColor(sf::Color::Yellow);
            else hpFg.setFillColor(sf::Color::Red);
            window.draw(hpBg); window.draw(hpFg);
            // === DRAW BUFF BARS ===
            // Рисуем только если бафф активен (ширина > 0)
            if (shieldBar.getSize().x > 0.f) {
                window.draw(shieldBar);
                window.draw(shieldLabel);
            }
            if (speedBar.getSize().x > 0.f) {
                window.draw(speedBar);
                window.draw(speedLabel);
            }
            if (rapidBar.getSize().x > 0.f) {
                window.draw(rapidBar);
                window.draw(rapidLabel);
            }
        } else if (state == GameState::MainMenu) {
            // Обновляем визуал кнопок
            updateButtonVisuals(startBtn, startText, menuSelected == 0);
            updateButtonVisuals(menuExitBtn, menuExitText, menuSelected == 1);
            
            // Рисуем меню
            window.draw(menuOverlay);
            window.draw(menuTitle);
            window.draw(startBtn); window.draw(startText);
            window.draw(menuExitBtn); window.draw(menuExitText);
            
        } else if (state == GameState::GameOver) {
            // Ваш существующий код отрисовки Game Over
            updateButtonVisuals(restartBtn, restartText, selectedButton == 0);
            updateButtonVisuals(exitBtn, exitText, selectedButton == 1);
            window.draw(overlay);
            window.draw(restartBtn); window.draw(restartText);
            window.draw(exitBtn);    window.draw(exitText);
            window.draw(goText);
        }

        window.display();
    }

    return 0;
}
