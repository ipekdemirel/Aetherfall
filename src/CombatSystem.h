#pragma once

class Player;
class Enemy;

class CombatSystem
{
public:
    CombatSystem();

    void Update(
        float deltaTime,
        Player& player,
        Enemy& enemy
    );

    bool ConsumeCameraShakeRequest();

private:
    void HandlePlayerAttack(
        Player& player,
        Enemy& enemy
    );

    void HandleEnemyAttack(
        float deltaTime,
        Player& player,
        Enemy& enemy
    );

    float playerAttackRange;
    float playerAttackDamage;
    float playerKnockbackForce;

    bool playerAttackWasActive;

    float enemyAttackRange;
    int enemyAttackDamage;

    float enemyAttackCooldown;
    float enemyAttackCooldownTimer;

    bool cameraShakeRequested;
};