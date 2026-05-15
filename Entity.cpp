#include "Entity.hpp"
#include <iostream>
#include <map>

// Глобальный кэш текстур (загружаем один раз на всю игру)
static std::map<std::string, std::shared_ptr<sf::Texture>> textureCache;

Entity::Entity() : speed(0.f), hp(1), halfSize(0.f, 0.f) {}

void Entity::setSprite(const std::string& texturePath, sf::Vector2f size) {
    // Кэшируем текстуру
    if (textureCache.find(texturePath) == textureCache.end()) {
        auto tex = std::make_shared<sf::Texture>();
        if (!tex->loadFromFile(texturePath)) {
            std::cerr << "Не удалось загрузить: " << texturePath << std::endl;
            return;
        }
        textureCache[texturePath] = tex;
    }

    // Создаём спрайт
    sprite = std::make_shared<sf::Sprite>(*textureCache[texturePath]);

    // Масштабируем под нужный размер
    sf::Vector2f texSize = sprite->getLocalBounds().getSize();
    sprite->setScale(size.x / texSize.x, size.y / texSize.y);

    // Origin в центре + сохраняем halfSize для коллизий
    sprite->setOrigin(size / 2.f);
    halfSize = size / 2.f;
}

sf::Vector2f Entity::getPosition() const {
    if (sprite) return sprite->getPosition();
    return shape.getPosition();
}

sf::Vector2f Entity::getHalfSize() const {
    return halfSize;  // Всегда актуально, независимо от спрайта/фигуры
}

sf::FloatRect Entity::getGlobalBounds() const {
    if (sprite) return sprite->getGlobalBounds();
    return shape.getGlobalBounds();
}

int Entity::getHP() const { return hp; }
bool Entity::isAlive() const { return hp > 0; }

void Entity::setPosition(const sf::Vector2f& pos) {
    if (sprite) sprite->setPosition(pos);
    else shape.setPosition(pos);
}

void Entity::move(const sf::Vector2f& offset) {
    if (sprite) sprite->move(offset);
    else shape.move(offset);
}

void Entity::takeDamage(int dmg) { hp -= dmg; }

void Entity::draw(sf::RenderWindow& window) const {
    if (sprite) window.draw(*sprite);
    else window.draw(shape);  // Фолбэк на прямоугольник
}
