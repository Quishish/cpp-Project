#include "Enemy.hpp"
#include <cmath>

Enemy::Enemy(float radius, float spd, int health, sf::Vector2f pos)
    : targetPosition(0.f, 0.f) {

    speed = spd;
    hp = health;
    shape.setRadius(radius);
    shape.setFillColor(sf::Color::Red);
    shape.setOrigin(radius, radius);
    shape.setPosition(pos);
}

void Enemy::update(float dt) {
    chaseTarget(targetPosition, dt);
}

void Enemy::chaseTarget(const sf::Vector2f& targetPos, float dt) {
    targetPosition = targetPos;
    sf::Vector2f dir = targetPos - getPosition();
    float dist = std::hypot(dir.x, dir.y);

    if (dist > 1.f) {
        sf::Vector2f normalizedDir = dir / dist;
        move(normalizedDir * speed * dt);
    }
}
