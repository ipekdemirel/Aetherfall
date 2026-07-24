#include "CombatSystem.h"

#include "Enemy.h"
#include "Player.h"

#include <raymath.h>

CombatSystem::CombatSystem()
    : playerAttackRange(2.5f),
    playerAttackDamage(25.0f),
    playerKnockbackForce(5.0f),
    playerAttackWasActive(false),
    enemyAttackRange(1.5f),
    enemyAttackDamage(10),
    enemyAttackCooldown(1.0f),
    enemyAttackCooldownTimer(0.0f),
    cameraShakeRequested(false)
{
}

void CombatSystem::Update(
    float deltaTime,
    Player& player,
    Enemy& enemy
)
{
    cameraShakeRequested = false;

    HandlePlayerAttack(
        player,
        enemy
    );

    HandleEnemyAttack(
        deltaTime,
        player,
        enemy
    );
}

void CombatSystem::HandlePlayerAttack(
    Player& player,
    Enemy& enemy
)
{
    const bool playerAttackIsActive =
        player.IsAttacking();

    if (
        playerAttackIsActive &&
        !playerAttackWasActive &&
        enemy.IsAlive()
        )
    {
        const Vector3 playerPosition =
            player.GetPosition();

        const Vector3 enemyPosition =
            enemy.GetPosition();

        const Vector3 playerToEnemy =
            Vector3Subtract(
                enemyPosition,
                playerPosition
            );

        const float distanceToEnemy =
            Vector3Length(playerToEnemy);

        if (distanceToEnemy <= playerAttackRange)
        {
            Vector3 directionToEnemy{
                0.0f,
                0.0f,
                0.0f
            };

            if (distanceToEnemy > 0.001f)
            {
                directionToEnemy =
                    Vector3Normalize(playerToEnemy);
            }

            const float facingAmount =
                Vector3DotProduct(
                    player.GetFacingDirection(),
                    directionToEnemy
                );

            if (facingAmount > 0.35f)
            {
                enemy.TakeDamage(
                    playerAttackDamage
                );

                enemy.ApplyKnockback(
                    directionToEnemy,
                    playerKnockbackForce
                );

                cameraShakeRequested = true;
            }
        }
    }

    playerAttackWasActive =
        playerAttackIsActive;
}

void CombatSystem::HandleEnemyAttack(
    float deltaTime,
    Player& player,
    Enemy& enemy
)
{
    if (enemyAttackCooldownTimer > 0.0f)
    {
        enemyAttackCooldownTimer -= deltaTime;

        if (enemyAttackCooldownTimer < 0.0f)
        {
            enemyAttackCooldownTimer = 0.0f;
        }
    }

    if (
        !player.IsAlive() ||
        !enemy.IsAlive()
        )
    {
        return;
    }

    const Vector3 playerPosition =
        player.GetPosition();

    const Vector3 enemyPosition =
        enemy.GetPosition();

    const Vector3 enemyToPlayer =
        Vector3Subtract(
            playerPosition,
            enemyPosition
        );

    const float distanceToPlayer =
        Vector3Length(enemyToPlayer);

    if (
        distanceToPlayer <= enemyAttackRange &&
        enemyAttackCooldownTimer <= 0.0f
        )
    {
        player.TakeDamage(
            enemyAttackDamage
        );

        enemyAttackCooldownTimer =
            enemyAttackCooldown;

        cameraShakeRequested = true;
    }
}

bool CombatSystem::ConsumeCameraShakeRequest()
{
    if (!cameraShakeRequested)
    {
        return false;
    }

    cameraShakeRequested = false;

    return true;
}