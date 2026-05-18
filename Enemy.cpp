#include "Enemy.hpp"
#include <cmath>

Enemy::Enemy(sf::Vector2f size, float spd, int health, sf::Vector2f pos)
    : targetPosition(0.f, 0.f)
{
    speed = spd;
    hp = health;

    // Пробуем загрузить спрайт
    setSprite("resources/sprites/Enemy.png", size);

    // Если спрайт не загрузился — используем красный прямоугольник
    if (!sprite) {
        shape.setSize(size);
        shape.setFillColor(sf::Color::Red);
        shape.setOrigin(size / 2.f);
        halfSize = size / 2.f;
    }

    setPosition(pos);
}

void Enemy::update(float dt) {
    chaseTarget(targetPosition, dt);
}

void Enemy::chaseTarget(const sf::Vector2f& targetPos, float dt) {
    targetPosition = targetPos;
    sf::Vector2f dir = targetPos - getPosition();
    float dist = std::hypot(dir.x, dir.y);

    if (dist > 1.f) {
        move(dir / dist * speed * dt);
    }
}
