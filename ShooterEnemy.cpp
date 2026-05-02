#include "ShooterEnemy.hpp"
#include <cmath>

const float PI = 3.1415926535f;

ShooterEnemy::ShooterEnemy(float radius, float spd, int health, sf::Vector2f pos)
    : Enemy(radius, spd, health, pos),
      shootTimer(2.0f), shootInterval(2.5f),
      bulletCount(12), bulletSpeed(200.f)
{
    shape.setFillColor(sf::Color::Magenta); // Визуально отличаем от обычных врагов
}

void ShooterEnemy::update(float dt, const sf::Vector2f& targetPos, std::vector<Bullet>& outBullets) {
    // Наследуем поведение преследования из базового класса
    chaseTarget(targetPos, dt);

    shootTimer -= dt;
    if (shootTimer <= 0.f) {
        shootTimer = shootInterval;
        fire(outBullets);
    }
}

void ShooterEnemy::fire(std::vector<Bullet>& outBullets) {
    for (int i = 0; i < bulletCount; ++i) {
        // Равномерно распределяем углы по кругу
        float angle = (i * 2.f * PI) / bulletCount;

        Bullet b(5.f, sf::Color::Red);
        b.shape.setPosition(getPosition());
        // Вектор скорости = направление * скорость
        b.velocity = {std::cos(angle) * bulletSpeed, std::sin(angle) * bulletSpeed};

        outBullets.push_back(b);
    }
}
