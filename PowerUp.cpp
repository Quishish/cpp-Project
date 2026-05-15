// powerUp.cpp
#include "powerUp.hpp"
#include <cstdlib>
#include <cmath>

PowerUp::PowerUp(float x, float y, Type type, float width, float height)
    : Entity(), type_(type), baseSize_(width, height)
{
    shape.setSize(baseSize_);
    shape.setOrigin(baseSize_ / 2.0f);
    shape.setPosition(x, y);
    shape.setOutlineThickness(2.0f);

    switch (type_) {
        case Type::Medkit:
            shape.setFillColor(sf::Color(255, 215, 0));
            shape.setOutlineColor(sf::Color(200, 150, 0));
            break;
        case Type::Shield:
            shape.setFillColor(sf::Color(30, 144, 255));
            shape.setOutlineColor(sf::Color(0, 100, 200));
            break;
        case Type::Speed:
            shape.setFillColor(sf::Color(50, 205, 50));   // зелёный
            shape.setOutlineColor(sf::Color(0, 150, 0));
            break;
        case Type::RapidFire:
            shape.setFillColor(sf::Color(255, 105, 180));  // розовый
            shape.setOutlineColor(sf::Color(200, 0, 100));
            break;
    }
    hp = 1;
    speed = 0.0f;
}

void PowerUp::update(float dt) { (void)dt; }

PowerUp::Type PowerUp::getType() const { return type_; }
int PowerUp::getHealAmount() const { return 1 + (std::rand() % 2); }

float PowerUp::getDuration() const {
    switch (type_) {
        case Type::Shield: return 3.0f;
        case Type::Speed:
        case Type::RapidFire: return 5.0f;
        default: return 0.0f;
    }
}

float PowerUp::getMultiplier() const {
    return (type_ == Type::Speed || type_ == Type::RapidFire) ? 1.5f : 1.0f;
}

sf::Vector2f PowerUp::generateRandomPosition(
    const sf::Vector2u& windowSize,
    const sf::FloatRect& playerBounds,
    float minDistance
) {
    sf::Vector2f pos;
    int attempts = 0;
    do {
        pos.x = 40.f + static_cast<float>(std::rand()) / RAND_MAX * (static_cast<float>(windowSize.x) - 80.f);
        pos.y = 40.f + static_cast<float>(std::rand()) / RAND_MAX * (static_cast<float>(windowSize.y) - 80.f);
        attempts++;
        sf::Vector2f pc(playerBounds.left + playerBounds.width/2.f, playerBounds.top + playerBounds.height/2.f);
        if (std::hypot(pos.x - pc.x, pos.y - pc.y) >= minDistance) break;
    } while (attempts < 30);
    return pos;
}