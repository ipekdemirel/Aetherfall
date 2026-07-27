#include "Enemy.h"

#include <raymath.h>

Enemy::Enemy(Vector3 startPosition)
    : position(startPosition),
    knockbackVelocity{ 0.0f, 0.0f, 0.0f },
    moveSpeed(2.5f),
    health(100.0f),
    maxHealth(100.0f),
    hitFlashTimer(0.0f),
    hitFlashDuration(0.25f),
    alive(true),
    scoreValue(100),
    scoreGiven(false)
{
}

void Enemy::Update(float deltaTime, Vector3 playerPosition)
{
    if (!alive)
    {
        return;
    }

    if (hitFlashTimer > 0.0f)
    {
        hitFlashTimer -= deltaTime;

        if (hitFlashTimer < 0.0f)
        {
            hitFlashTimer = 0.0f;
        }
    }

    position.x += knockbackVelocity.x * deltaTime;
    position.z += knockbackVelocity.z * deltaTime;

    knockbackVelocity = Vector3Scale(
        knockbackVelocity,
        0.88f
    );

    if (Vector3Length(knockbackVelocity) < 0.05f)
    {
        knockbackVelocity = { 0.0f, 0.0f, 0.0f };
    }

    Vector3 direction = Vector3Subtract(
        playerPosition,
        position
    );

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

    const Color bodyColor =
        hitFlashTimer > 0.0f
        ? Color{ 255, 255, 180, 255 }
    : RED;

    DrawCube(
        position,
        1.0f,
        2.0f,
        1.0f,
        bodyColor
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
    hitFlashTimer = hitFlashDuration;

    if (health < 0.0f)
    {
        health = 0.0f;
    }

    if (health <= 0.0f)
    {
        alive = false;
    }
}

void Enemy::ApplyKnockback(Vector3 direction, float force)
{
    if (!alive || Vector3Length(direction) <= 0.0f)
    {
        return;
    }

    knockbackVelocity = Vector3Scale(
        Vector3Normalize(direction),
        force
    );
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

// ==========================
// SCORE SYSTEM
// ==========================

int Enemy::GetScoreValue() const
{
    return scoreValue;
}

bool Enemy::ScoreAlreadyGiven() const
{
    return scoreGiven;
}

void Enemy::MarkScoreGiven()
{
    scoreGiven = true;
}