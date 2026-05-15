#include "Player.hpp"

Player::Player(sf::Vector2f size, float spd, int health, sf::Vector2f pos)
    : movementInput(0.f, 0.f)
{
    speed = spd;
    hp = health;

    shape.setSize(size);
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin(size / 2.f);  // Центр в середине квадрата

    halfSize = size / 2.f;        // Сохраняем для коллизий и границ
    shape.setPosition(pos);
}

void Player::update(float dt) {
    if (movementInput.x != 0.f || movementInput.y != 0.f) {
        move(movementInput * speed * dt);
    }
}

void Player::setMovementInput(const sf::Vector2f& input) {
    movementInput = input;
}
