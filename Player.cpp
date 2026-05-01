#include "Player.hpp"
#include <cmath>

Player::Player(float radius, float spd, int health, sf::Vector2f pos)
    : movementInput(0.f, 0.f) {

    speed = spd;
    hp = health;
    shape.setRadius(radius);
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin(radius, radius);
    shape.setPosition(pos);
}

void Player::update(float dt) {
    if (movementInput.x != 0.f || movementInput.y != 0.f) {
        // Нормализация уже сделана в setMovementInput
        move(movementInput * speed * dt);
    }
}

void Player::handleInput(float dt) {
    // Этот метод можно использовать для внутренней обработки,
    // но лучше задавать input извне (из main)
}

void Player::setMovementInput(const sf::Vector2f& input) {
    movementInput = input;
}
