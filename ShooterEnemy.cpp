#include "ShooterEnemy.hpp"
#include <cmath>

const float PI = 3.1415926535f;

ShooterEnemy::ShooterEnemy(sf::Vector2f size, float spd, int health, sf::Vector2f pos)
    : Enemy(size, spd, health, pos),  // ѕередаЄм size в базовый класс
      shootTimer(2.0f), shootInterval(2.5f),
      bulletCount(12), bulletSpeed(200.f)
{
    shape.setFillColor(sf::Color::Magenta);
}

void ShooterEnemy::update(float dt, const sf::Vector2f& targetPos, std::vector<Bullet>& outBullets) {
    chaseTarget(targetPos, dt);
    shootTimer -= dt;
    if (shootTimer <= 0.f) {
        shootTimer = shootInterval;
        fire(outBullets);
    }
}

void ShooterEnemy::fire(std::vector<Bullet>& outBullets) {
    for (int i = 0; i < bulletCount; ++i) {
        float angle = (i * 2.f * PI) / bulletCount;
        Bullet b({10.f, 10.f}, sf::Color::Red); //  вадратна€ пул€
        b.shape.setPosition(getPosition());
        b.velocity = {std::cos(angle) * bulletSpeed, std::sin(angle) * bulletSpeed};
        outBullets.push_back(b);
    }
}
