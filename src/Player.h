#pragma once

#include <raylib.h>

#include "Weapon.h"

class Player
{
public:
    Player();

    void Update(float deltaTime);
    void Draw() const;

    Vector3 GetPosition() const;
    Vector3 GetFacingDirection() const;

    bool IsAttacking() const;
    float GetAttackProgress() const;

    void TakeDamage(int damage);
    int GetHealth() const;
    int GetMaxHealth() const;
    void Heal(int amount);
    bool IsAlive() const;

    void IncreaseMaxHealth(int amount);
    void IncreaseArmor(float amount);
    void IncreaseMovementSpeed(float walkAmount, float runAmount);
    void ReduceDashCooldown(float amount);

    float GetArmor() const;
    float GetWalkSpeed() const;
    float GetRunSpeed() const;
    float GetDashCooldown() const;

private:
    Vector3 position;
    Vector3 facingDirection;

    float walkSpeed;
    float runSpeed;

    float dashSpeed;
    float dashDuration;
    float dashTimer;

    float dashCooldown;
    float dashCooldownTimer;

    bool isDashing;

    Weapon weapon;

    int health;
    int maxHealth;
    float armor;
};
