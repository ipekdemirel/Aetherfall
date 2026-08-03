#pragma once

#include <raylib.h>

class Enemy
{
public:
    Enemy(Vector3 startPosition);

    void Update(
        float deltaTime,
        Vector3 playerPosition,
        Vector3 separationForce,
        bool forceChaseTarget = false
    );

    void Draw() const;

    void TakeDamage(float damage);

    void ApplyKnockback(
        Vector3 direction,
        float force
    );

    bool IsAlive() const;

    Vector3 GetPosition() const;

    float GetHealth() const;
    float GetMaxHealth() const;

    bool IsAlerted() const;

    // ==========================
    // SCORE SYSTEM
    // ==========================

    int GetScoreValue() const;

    bool ScoreAlreadyGiven() const;

    void MarkScoreGiven();

private:
    Vector3 position;
    Vector3 homePosition;
    Vector3 knockbackVelocity;
    Vector3 facingDirection;

    float moveSpeed;
    float returnSpeed;

    float health;
    float maxHealth;

    float hitFlashTimer;
    float hitFlashDuration;
    float visualTime;
    float deathEffectTimer;
    float deathEffectDuration;

    // ==========================
    // DETECTION SYSTEM
    // ==========================

    float detectionRange;
    float loseTargetRange;

    float forgetDuration;
    float forgetTimer;

    float homeStopDistance;

    bool alerted;
    bool alive;

    // ==========================
    // SCORE SYSTEM
    // ==========================

    int scoreValue;

    bool scoreGiven;
};
