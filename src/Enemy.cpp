#include "Enemy.h"

#include <raymath.h>

Enemy::Enemy(Vector3 startPosition)
    : position(startPosition),
    moveSpeed(2.5f),
    health(100.0f),
    maxHealth(100.0f),
    alive(true)
{
}

void Enemy::Update(float deltaTime, Vector3 playerPosition)
{
    if (!alive)
    {
        return;
    }

    Vector3 direction = Vector3Subtract(playerPosition, position);

    if (Vector3Length(direction) > 0.1f)
    {
        direction = Vector3Normalize(direction);

        position.x += direction.x * moveSpeed * deltaTime;
        position.z += direction.z * moveSpeed * deltaTime;
    }
}

void Enemy::Draw() const
{
    if (!alive)
    {
        return;
    }

    DrawCube(
        position,
        1.0f,
        2.0f,
        1.0f,
        RED
    );

    DrawCubeWires(
        position,
        1.0f,
        2.0f,
        1.0f,
        MAROON
    );
}

void Enemy::TakeDamage(float damage)
{
    if (!alive)
    {
        return;
    }

    health -= damage;

    if (health < 0.0f)
    {
        health = 0.0f;
    }

    if (health <= 0.0f)
    {
        alive = false;
    }
}

bool Enemy::IsAlive() const
{
    return alive;
}

Vector3 Enemy::GetPosition() const
{
    return position;
}

float Enemy::GetHealth() const
{
    return health;
}

float Enemy::GetMaxHealth() const
{
    return maxHealth;
}