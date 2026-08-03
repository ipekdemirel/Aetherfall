#pragma once

#include <raylib.h>

class Player;

class Boss
{
public:
    explicit Boss(Vector3 startPosition);

    void Update(float deltaTime, Player& player);
    void Draw() const;

    void TakeDamage(float damage);
    void ApplyKnockback(Vector3 direction, float force);

    bool IsAlive() const;
    Vector3 GetPosition() const;
    float GetHealth() const;
    float GetMaxHealth() const;
    bool IsRageModeActive() const;
    bool ConsumeAttackHitRequest();

private:
    enum class AttackType
    {
        None,
        Melee,
        MeteorStrike,
        RageSlam
    };

    void UpdateHitFlash(float deltaTime);
    void UpdateKnockback(float deltaTime);
    void UpdateMovement(float deltaTime, Vector3 playerPosition);
    void UpdateAttack(float deltaTime, Player& player);

    void StartMeleeAttack();
    void StartMeteorStrike(Vector3 playerPosition);
    void StartRageSlamAttack();

    void ResolveMeleeAttack(Player& player);
    void ResolveMeteorStrike(Player& player);
    void ResolveRageSlamAttack(Player& player);

    void DrawAttackWarning() const;

    Vector3 position;
    Vector3 knockbackVelocity;
    Vector3 facingDirection;
    Vector3 attackTarget;

    float normalMoveSpeed;
    float rageMoveSpeed;

    float health;
    float maxHealth;

    float attackRange;
    int attackDamage;

    float attackCooldown;
    float attackCooldownTimer;

    float hitFlashTimer;
    float hitFlashDuration;
    float animationTime;

    bool alive;
    bool rageModeActive;
    bool attackHitRequested;

    AttackType currentAttack;
    float attackWindupTimer;

    float meleeWindupDuration;
    float rageMeleeWindupDuration;

    float meteorWindupDuration;
    float meteorRange;
    int meteorDamage;

    float rageSlamWindupDuration;
    float rageSlamRange;
    int rageSlamDamage;

    int attacksSinceLastSlam;
};