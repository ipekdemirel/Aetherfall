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

    // ==========================
    // HEALTH
    // ==========================
    void TakeDamage(int damage);

    int GetHealth() const;
    int GetMaxHealth() const;

    bool IsAlive() const;

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

    // ==========================
    // PLAYER HEALTH
    // ==========================
    int health;
    int maxHealth;
};