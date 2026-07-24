#pragma once

#include <raylib.h>

class Enemy
{
public:
    Enemy(Vector3 startPosition);

    void Update(float deltaTime, Vector3 playerPosition);
    void Draw() const;

    void TakeDamage(float damage);

    // Yeni
    void ApplyKnockback(Vector3 direction, float force);

    bool IsAlive() const;

    Vector3 GetPosition() const;

    float GetHealth() const;
    float GetMaxHealth() const;

private:
    Vector3 position;

    // Yeni
    Vector3 knockbackVelocity;

    float moveSpeed;
    float health;
    float maxHealth;

    float hitFlashTimer;
    float hitFlashDuration;

    bool alive;
};