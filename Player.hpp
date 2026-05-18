#pragma once
#include "Entity.hpp"

class Player : public Entity {
public:
    Player(sf::Vector2f size, float spd, int health, sf::Vector2f pos);


    void update(float dt) override;


    void handleInput(float dt);
    void setMovementInput(const sf::Vector2f& input);

private:
    sf::Vector2f movementInput;
};
