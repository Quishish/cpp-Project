#pragma once
#include "Entity.hpp"

class Enemy : public Entity {
public:
    Enemy(float radius, float spd, int health, sf::Vector2f pos);

    // Реализация виртуальной функции
    void update(float dt) override;

    // Специфичные для врага методы
    void chaseTarget(const sf::Vector2f& targetPos, float dt);

private:
    sf::Vector2f targetPosition;
};
