#pragma once
#include "Entity.hpp"

class Player : public Entity {
public:
    Player(float radius, float spd, int health, sf::Vector2f pos);

    // Реализация виртуальной функции
    void update(float dt) override;

    // Специфичные для игрока методы
    void handleInput(float dt);
    void setMovementInput(const sf::Vector2f& input);

private:
    sf::Vector2f movementInput;
};
