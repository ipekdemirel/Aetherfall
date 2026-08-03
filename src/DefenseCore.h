#pragma once

#include <vector>

#include <raylib.h>

class Enemy;

class DefenseCore
{
public:
    DefenseCore();

    void Reset(Vector3 startPosition);

    bool Update(
        float deltaTime,
        const std::vector<Enemy>& enemies
    );

    void Draw() const;
    void DrawHUD() const;

    Vector3 GetPosition() const;

    float GetHealth() const;
    float GetMaxHealth() const;

    bool IsAlive() const;

private:
    Vector3 position;

    float health;
    float maxHealth;

    float enemyAttackRange;
    float damagePerEnemy;
    float damageInterval;
    float damageTimer;
    float protectionTimer;

    int maxSimultaneousAttackers;
    int activeAttackerCount;

    float visualTime;
    float hitFlashTimer;
};
