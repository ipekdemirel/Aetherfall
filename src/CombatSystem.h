#pragma once

#include <vector>

class Player;
class Enemy;
class Boss;

class CombatSystem
{
public:
    CombatSystem();

    // Normal enemy combat
    void Update(
        float deltaTime,
        Player& player,
        std::vector<Enemy>& enemies
    );

    // Boss combat
    void UpdateBoss(
        float deltaTime,
        Player& player,
        Boss& boss
    );

    bool ConsumeCameraShakeRequest();

    void IncreasePlayerAttackDamage(
        float amount
    );

    float GetPlayerAttackDamage() const;

private:
    void HandlePlayerAttack(
        Player& player,
        std::vector<Enemy>& enemies
    );

    void HandlePlayerAttackBoss(
        Player& player,
        Boss& boss
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