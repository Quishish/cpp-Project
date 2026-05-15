#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

class PowerUp : public Entity {
public:
    enum class Type {
        Medkit,     // лечение +1~2 HP
        Shield,     // неуязвимость 3 сек
        Speed,      // скорость ×1.5, 5 сек
        RapidFire   // скорострельность ×1.5, 5 сек
    };

    PowerUp(float x, float y, Type type, float width = 24.0f, float height = 24.0f);
    void update(float dt) override;

    Type getType() const;
    int getHealAmount() const;
    float getDuration() const;
    float getMultiplier() const;

    static sf::Vector2f generateRandomPosition(
        const sf::Vector2u& windowSize,
        const sf::FloatRect& playerBounds,
        float minDistance
    );

private:
    Type type_;
    sf::Vector2f baseSize_;
};