#include "powerUp.hpp"
#include <cstdlib>
#include <cmath>
#include <string>

// -------------------------------------------------------------------------
// Конструктор
// -------------------------------------------------------------------------
PowerUp::PowerUp(float x, float y, Type type, float width, float height)
: Entity(), type_(type), baseSize_(width, height) {
    // 1. Настраиваем прямоугольник-заглушку (fallback)
    shape.setSize(baseSize_);
    shape.setOrigin(baseSize_ / 2.0f);
    shape.setPosition(x, y);
    shape.setOutlineThickness(2.0f);

    // 2. Определяем путь к спрайту и цвета
    std::string spritePath;
    sf::Color fillColor, outlineColor;

    switch (type_) {
        case Type::Medkit:
            spritePath = "resources/sprites/Health.png";
            fillColor = sf::Color(255, 215, 0);   // Золотой
            outlineColor = sf::Color(200, 150, 0);
            break;
        case Type::Shield:
            spritePath = "resources/sprites/Shield.png";
            fillColor = sf::Color(30, 144, 255);  // Синий
            outlineColor = sf::Color(0, 100, 200);
            break;
        case Type::Speed:
            spritePath = "resources/sprites/Speed.png";
            fillColor = sf::Color(50, 205, 50);   // Зелёный
            outlineColor = sf::Color(0, 150, 0);
            break;
        case Type::RapidFire:
            spritePath = "resources/sprites/Rapid.png";
            fillColor = sf::Color(255, 105, 180); // Розовый
            outlineColor = sf::Color(200, 0, 100);
            break;
    }

    shape.setFillColor(fillColor);
    shape.setOutlineColor(outlineColor);

    // 3. Пытаемся загрузить спрайт через базовый класс
    // setSprite() сам масштабирует текстуру под baseSize_ и установит origin в центр
    setSprite(spritePath, baseSize_);

    // Инициализация полей Entity
    hp = 1;
    speed = 0.0f;
}

// -------------------------------------------------------------------------
// Остальные методы без изменений
// -------------------------------------------------------------------------
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
