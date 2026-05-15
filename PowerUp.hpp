#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

class PowerUp: public Entity {
public:
    enum class Type {
        Medkit,
    };

    PowerUp(
        float x,
        float y,
        Type type = Type::Medkit,
        float width = 24.0f,
        float height = 24.0f
    );

    void update(float dt) override;

    Type getType() const;
    int getHealAmount() const;

    static sf::Vector2f generateRandomPosition(
        const sf::Vector2u& windowSize,
        const sf::FloatRect& playerBounds,
        float minDistance
    );

private:
    Type type_;                     // тип павер-апа
    sf::Vector2f baseSize_;         // базовый размер (ширина, высота)
};