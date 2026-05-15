#include "Bullet.hpp"

Bullet::Bullet(sf::Vector2f size, sf::Color color) {
    shape.setSize(size);
    shape.setFillColor(color);
    shape.setOrigin(size / 2.f);
    halfSize = size / 2.f;
}

void Bullet::update(float dt) { shape.move(velocity * dt); }

bool Bullet::isOffScreen(unsigned int windowWidth, unsigned int windowHeight) const {
    sf::FloatRect bounds = shape.getGlobalBounds();
    return (bounds.left + bounds.width < 0 || bounds.left > static_cast<float>(windowWidth) ||
            bounds.top + bounds.height < 0 || bounds.top > static_cast<float>(windowHeight));
}
