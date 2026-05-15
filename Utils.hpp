#pragma once
#include "Entity.hpp"
#include "Bullet.hpp"

// Коллизия между двумя прямоугольными сущностями
inline bool checkCollision(const Entity& a, const Entity& b) {
    return a.getGlobalBounds().intersects(b.getGlobalBounds());
}

// Коллизия пули с сущностью
inline bool checkBulletCollision(const sf::RectangleShape& bullet, const Entity& entity) {
    return bullet.getGlobalBounds().intersects(entity.getGlobalBounds());
}
