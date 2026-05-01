#pragma once
#include <SFML/Graphics.hpp>

class Entity {
protected:
    sf::CircleShape shape;
    float speed;
    int hp;

public:
    Entity();
    virtual ~Entity() = default;

    // Чистые виртуальные функции (интерфейс)
    virtual void update(float dt) = 0;

    // Геттеры
    sf::Vector2f getPosition() const;
    float getRadius() const;
    sf::FloatRect getGlobalBounds() const;
    int getHP() const;
    bool isAlive() const;

    // Сеттеры и методы
    void setPosition(const sf::Vector2f& pos);
    void move(const sf::Vector2f& offset);
    void takeDamage(int dmg);
    void draw(sf::RenderWindow& window) const;
};
