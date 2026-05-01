#pragma once
#include "Entity.hpp"
#include <cmath>

inline bool checkCollision(const Entity& a, const Entity& b) {
    sf::Vector2f diff = a.getPosition() - b.getPosition();
    float distance = std::hypot(diff.x, diff.y);
    return distance < (a.getRadius() + b.getRadius());
}

inline bool checkBulletCollision(const sf::CircleShape& bullet, const Entity& entity) {
    sf::Vector2f diff = bullet.getPosition() - entity.getPosition();
    float distance = std::hypot(diff.x, diff.y);
    return distance < (bullet.getRadius() + entity.getRadius());
}
