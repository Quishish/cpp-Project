#pragma once
#include <SFML/Graphics.hpp>

struct Bullet {
    sf::CircleShape shape;
    sf::Vector2f velocity;

    Bullet(float radius = 5.f, sf::Color color = sf::Color::Yellow);
    void update(float dt);
    bool isOffScreen(unsigned int windowWidth, unsigned int windowHeight) const;
};
