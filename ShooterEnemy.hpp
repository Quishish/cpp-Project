#pragma once
#include "Enemy.hpp"
#include "Bullet.hpp"
#include <vector>

class ShooterEnemy : public Enemy {
public:
    ShooterEnemy(float radius, float spd, int health, sf::Vector2f pos);

    // Обновляем состояние и генерируем пули в переданный вектор
    void update(float dt, const sf::Vector2f& targetPos, std::vector<Bullet>& outBullets);

private:
    float shootTimer;
    float shootInterval;
    int bulletCount;
    float bulletSpeed;

    void fire(std::vector<Bullet>& outBullets);
};
