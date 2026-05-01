#include "Entity.hpp"

Entity::Entity() : speed(0.f), hp(1) {}

sf::Vector2f Entity::getPosition() const {
    return shape.getPosition();
}

float Entity::getRadius() const {
    return shape.getRadius();
}

sf::FloatRect Entity::getGlobalBounds() const {
    return shape.getGlobalBounds();
}

int Entity::getHP() const {
    return hp;
}

bool Entity::isAlive() const {
    return hp > 0;
}

void Entity::setPosition(const sf::Vector2f& pos) {
    shape.setPosition(pos);
}

void Entity::move(const sf::Vector2f& offset) {
    shape.move(offset);
}

void Entity::takeDamage(int dmg) {
    hp -= dmg;
}

void Entity::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}
