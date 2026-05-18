#include "Player.hpp"
#include <cmath>

Player::Player(sf::Vector2f size, float spd, int health, sf::Vector2f pos)
    : movementInput(0.f, 0.f) {

    speed = spd;
    hp = health;

    // Пробуем загрузить спрайт
    setSprite("resources/sprites/Player.png", size);

    // Если спрайт не загрузился — настраиваем прямоугольник
    if (!sprite) {
        shape.setSize(size);
        shape.setFillColor(sf::Color::Green);
        shape.setOrigin(size / 2.f);
        halfSize = size / 2.f;
    }

    setPosition(pos);
}

void Player::update(float dt) {
    if (movementInput.x != 0.f || movementInput.y != 0.f) {
        move(movementInput * speed * dt);
    }
}

void Player::setMovementInput(const sf::Vector2f& input) {
    movementInput = input;
}
