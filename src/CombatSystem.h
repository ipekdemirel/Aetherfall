#pragma once

#include <vector>

class Player;
class Enemy;

class CombatSystem
{
public:
    CombatSystem();

    void Update(
        float deltaTime,
        Player& player,
        std::vector<Enemy>& enemies
    );

    bool ConsumeCameraShakeRequest();

    // ==========================
    // PLAYER DAMAGE UPGRADE
    // ==========================

    void IncreasePlayerAttackDamage(
        float amount
    );

    float GetPlayerAttackDamage() const;

private:
    void HandlePlayerAttack(
        Player& player,
        std::vector<Enemy>& enemies
    );

    void HandleEnemyAttacks(
        float deltaTime,
        Player& player,
        std::vector<Enemy>& enemies
    );

    float playerAttackRange;
    float playerAttackDamage;
    float playerKnockbackForce;

    bool playerAttackWasActive;

    float enemyAttackRange;
    int enemyAttackDamage;
    float enemyAttackCooldown;

    std::vector<float> enemyAttackCooldownTimers;

    bool cameraShakeRequested;
};