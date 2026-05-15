#include "powerUp.hpp"
#include <cstdlib>
#include <cmath>

// -------------------------------------------------------------------------
// Конструктор
// -------------------------------------------------------------------------
PowerUp::PowerUp(float x, float y, Type type, float width, float height)
    : Entity(),
      type_(type),
      baseSize_(width, height)
{
    // Настраиваем прямоугольный хитбокс через base-класс
    shape.setSize(baseSize_);
    shape.setOrigin(baseSize_ / 2.0f);  // центр в центре
    shape.setPosition(x, y);

    // Визуальное оформление: золотой/жёлтый
    shape.setFillColor(sf::Color(255, 215, 0));
    shape.setOutlineColor(sf::Color(200, 150, 0));
    shape.setOutlineThickness(2.0f);

    // Инициализация полей базового класса
    hp = 1;
    speed = 0.0f;
}

// -------------------------------------------------------------------------
// Обновление: визуальная пульсация (не влияет на хитбокс)
// -------------------------------------------------------------------------
void PowerUp::update(float dt) {
    (void) dt;
}
// -------------------------------------------------------------------------
// Геттер типа
// -------------------------------------------------------------------------
PowerUp::Type PowerUp::getType() const {
    return type_;
}

// -------------------------------------------------------------------------
// Геттер лечения: 1 или 2 случайно
// -------------------------------------------------------------------------
int PowerUp::getHealAmount() const {
    return 1 + (std::rand() % 2);
}

// -------------------------------------------------------------------------
// Генерация позиции: не слишком близко к игроку
// -------------------------------------------------------------------------
sf::Vector2f PowerUp::generateRandomPosition(
    const sf::Vector2u& windowSize,
    const sf::FloatRect& playerBounds,
    float minDistance
) {
    sf::Vector2f pos;
    int attempts = 0;
    const int maxAttempts = 30;

    do {
        // Позиция с отступом от краёв окна
        pos.x = 40.0f + static_cast<float>(std::rand()) / RAND_MAX *
                (static_cast<float>(windowSize.x) - 80.0f);
        pos.y = 40.0f + static_cast<float>(std::rand()) / RAND_MAX *
                (static_cast<float>(windowSize.y) - 80.0f);
        attempts++;

        // Центр игрока для проверки дистанции
        sf::Vector2f playerCenter(
            playerBounds.left + playerBounds.width / 2.0f,
            playerBounds.top + playerBounds.height / 2.0f
        );
        float dist = std::hypot(pos.x - playerCenter.x, pos.y - playerCenter.y);

        if (dist >= minDistance) {
            break;
        }
    } while (attempts < maxAttempts);

    return pos;
}