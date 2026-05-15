#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

class Entity {
protected:
    std::shared_ptr<sf::Sprite> sprite;  // <-- �����: ������������ ������
    sf::RectangleShape shape;
    float speed;
    int hp;
    sf::Vector2f halfSize;

public:
    Entity();
    virtual ~Entity() = default;

    virtual void update(float dt) = 0;

    // �������
    sf::Vector2f getPosition() const;
    sf::Vector2f getHalfSize() const;
    sf::FloatRect getGlobalBounds() const;
    int getHP() const;
    bool isAlive() const;

    // ������
    void setSprite(const std::string& texturePath, sf::Vector2f size);  // <-- �����
    void setPosition(const sf::Vector2f& pos);
    void move(const sf::Vector2f& offset);
    void takeDamage(int dmg);
    void draw(sf::RenderWindow& window) const;
    void setSpeed(float newSpeed);
    float getBaseSpeed() const { return speed; }
};
