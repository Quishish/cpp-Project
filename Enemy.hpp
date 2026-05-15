#pragma once
#include "Entity.hpp"

class Enemy : public Entity {
public:
    Enemy(sf::Vector2f size, float spd, int health, sf::Vector2f pos);

    // Реализация виртуальной функции
    void update(float dt) override;

    // Специфичные для врага методы
    void chaseTarget(const sf::Vector2f& targetPos, float dt);

private:
    sf::Vector2f targetPosition;
};
