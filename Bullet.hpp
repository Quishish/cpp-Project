#pragma once
#include <SFML/Graphics.hpp>

struct Bullet {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    sf::Vector2f halfSize;  // <-- Для коллизий

    Bullet(sf::Vector2f size = {10.f, 10.f}, sf::Color color = sf::Color::Yellow);
    void update(float dt);
    bool isOffScreen(unsigned int windowWidth, unsigned int windowHeight) const;
};
