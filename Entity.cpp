#include "Entity.hpp"
#include <iostream>
#include <map>

// ���������� ��� ������� (��������� ���� ��� �� ��� ����)
static std::map<std::string, std::shared_ptr<sf::Texture>> textureCache;

Entity::Entity() : speed(0.f), hp(1), halfSize(0.f, 0.f) {}

void Entity::setSprite(const std::string& texturePath, sf::Vector2f size) {
    if (textureCache.find(texturePath) == textureCache.end()) {
        auto tex = std::make_shared<sf::Texture>();
        if (!tex->loadFromFile(texturePath)) {
            std::cerr << "⚠️ Не удалось загрузить: " << texturePath << std::endl;
            return;
        }
        textureCache[texturePath] = tex;
    }

    sprite = std::make_shared<sf::Sprite>(*textureCache[texturePath]);
    sf::Vector2f texSize = sprite->getLocalBounds().getSize();
    sprite->setScale(size.x / texSize.x, size.y / texSize.y);
    sprite->setOrigin(size / 2.f);
    halfSize = size / 2.f;

    // ✅ КЛЮЧЕВОЕ: копируем позицию из shape в новый спрайт
    sprite->setPosition(shape.getPosition());
}

sf::Vector2f Entity::getPosition() const {
    if (sprite) return sprite->getPosition();
    return shape.getPosition();
}

sf::Vector2f Entity::getHalfSize() const {
    return halfSize;  // ������ ���������, ���������� �� �������/������
}

sf::FloatRect Entity::getGlobalBounds() const {
    if (sprite) return sprite->getGlobalBounds();
    return shape.getGlobalBounds();
}

int Entity::getHP() const { return hp; }
bool Entity::isAlive() const { return hp > 0; }

void Entity::setPosition(const sf::Vector2f& pos) {
    shape.setPosition(pos);          // Обновляем всегда (для коллизий)
    if (sprite) sprite->setPosition(pos); // Обновляем спрайт, если он есть
}

void Entity::move(const sf::Vector2f& offset) {
    if (sprite) sprite->move(offset);
    else shape.move(offset);
}

void Entity::takeDamage(int dmg) { hp -= dmg; }

void Entity::draw(sf::RenderWindow& window) const {
    if (sprite) window.draw(*sprite);
    else window.draw(shape);  // ������ �� �������������
}

void Entity::setSpeed(float newSpeed) {
    speed = newSpeed;
}
