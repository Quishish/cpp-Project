#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath> 
int main() {
    const unsigned int WIDTH = 800;
    const unsigned int HEIGHT = 600;
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Bullet Hell Base");
    window.setFramerateLimit(60); 

    sf::CircleShape player(15.f);
    player.setFillColor(sf::Color::Green);
    player.setOrigin(player.getRadius(), player.getRadius()); 
    player.setPosition(WIDTH / 2.f, HEIGHT / 2.f);

    const float PLAYER_SPEED = 250.f;
    sf::Clock deltaClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float dt = deltaClock.restart().asSeconds();

        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) movement.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) movement.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) movement.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) movement.x += 1.f;

        if (movement.x != 0.f || movement.y != 0.f) {
            float length = std::hypot(movement.x, movement.y);
            movement /= length;
        }

        player.move(movement * PLAYER_SPEED * dt);

        sf::FloatRect bounds = player.getGlobalBounds();
        float halfSize = player.getRadius();
        sf::Vector2f pos = player.getPosition();

        pos.x = std::clamp(pos.x, halfSize, static_cast<float>(WIDTH - halfSize));
        pos.y = std::clamp(pos.y, halfSize, static_cast<float>(HEIGHT - halfSize));
        player.setPosition(pos);

        window.clear(sf::Color(20, 20, 30));
        window.draw(player);
        window.display();
    }

    return 0;
}
