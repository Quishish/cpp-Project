#include <SFML/Graphics.hpp>
#include <algorithm> // для std::clamp (C++17) или можно заменить на if
#include <cmath> // <-- Добавить эту строку

int main() {
    // 1. Настройка окна
    const unsigned int WIDTH = 800;
    const unsigned int HEIGHT = 600;
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Bullet Hell Base");
    window.setFramerateLimit(60); // Ограничение кадров для стабильности

    // 2. Игрок
    sf::CircleShape player(15.f);
    player.setFillColor(sf::Color::Green);
    player.setOrigin(player.getRadius(), player.getRadius()); // Центр в середине
    player.setPosition(WIDTH / 2.f, HEIGHT / 2.f);

    // Параметры движения
    const float PLAYER_SPEED = 250.f; // пикселей в секунду
    sf::Clock deltaClock;

    // 3. Игровой цикл
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Delta time для независимости от FPS
        float dt = deltaClock.restart().asSeconds();

        // 4. Обработка ввода (WASD)
        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) movement.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) movement.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) movement.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) movement.x += 1.f;

        // Нормализация вектора (чтобы по диагонали не было быстрее)
        if (movement.x != 0.f || movement.y != 0.f) {
            float length = std::hypot(movement.x, movement.y);
            movement /= length;
        }

        player.move(movement * PLAYER_SPEED * dt);

        // 5. Ограничение границами окна
        sf::FloatRect bounds = player.getGlobalBounds();
        float halfSize = player.getRadius();
        sf::Vector2f pos = player.getPosition();

        pos.x = std::clamp(pos.x, halfSize, static_cast<float>(WIDTH - halfSize));
        pos.y = std::clamp(pos.y, halfSize, static_cast<float>(HEIGHT - halfSize));
        player.setPosition(pos);

        // 6. Рендеринг
        window.clear(sf::Color(20, 20, 30)); // Тёмный фон, как в классических данмаку
        window.draw(player);
        window.display();
    }

    return 0;
}
