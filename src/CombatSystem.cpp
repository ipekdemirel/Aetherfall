#include "CombatSystem.h"

#include "Boss.h"
#include "Enemy.h"
#include "Player.h"

#include <raymath.h>

CombatSystem::CombatSystem()
    :
    playerAttackRange(2.5f),
    playerAttackDamage(25.0f),
    playerKnockbackForce(5.0f),
    playerAttackWasActive(false),
    enemyAttackRange(1.5f),
    enemyAttackDamage(10),
    enemyAttackCooldown(1.0f),
    cameraShakeRequested(false)
{
}

void CombatSystem::Update(
    float deltaTime,
    Player& player,
    std::vector<Enemy>& enemies
)
{
    cameraShakeRequested = false;

    HandlePlayerAttack(
        player,
        enemies
    );

    HandleEnemyAttacks(
        deltaTime,
        player,
        enemies
    );
}

void CombatSystem::UpdateBoss(
    float deltaTime,
    Player& player,
    Boss& boss
)
{
    cameraShakeRequested = false;

    boss.Update(
        deltaTime,
        player
    );

    HandlePlayerAttackBoss(
        player,
        boss
    );

    if (boss.ConsumeAttackHitRequest())
    {
        cameraShakeRequested = true;
    }
}

void CombatSystem::HandlePlayerAttack(
    Player& player,
    std::vector<Enemy>& enemies
)
{
    const bool playerAttackIsActive =
        player.IsAttacking();

    if (
        playerAttackIsActive &&
        !playerAttackWasActive
        )
    {
        const Vector3 playerPosition =
            player.GetPosition();

        for (Enemy& enemy : enemies)
        {
            if (!enemy.IsAlive())
            {
                continue;
            }

            const Vector3 enemyPosition =
                enemy.GetPosition();

            const Vector3 playerToEnemy =
                Vector3Subtract(
                    enemyPosition,
                    playerPosition
                );

            const float distanceToEnemy =
                Vector3Length(
                    playerToEnemy
                );

            if (
                distanceToEnemy >
                playerAttackRange
                )
            {
                continue;
            }

            Vector3 directionToEnemy{
                0.0f,
                0.0f,
                0.0f
            };

            if (distanceToEnemy > 0.001f)
            {
                directionToEnemy =
                    Vector3Normalize(
                        playerToEnemy
                    );
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

void CombatSystem::HandlePlayerAttackBoss(
    Player& player,
    Boss& boss
)
{
    const bool playerAttackIsActive =
        player.IsAttacking();

    if (
        playerAttackIsActive &&
        !playerAttackWasActive &&
        boss.IsAlive()
        )
    {
        const Vector3 playerPosition =
            player.GetPosition();

        const Vector3 bossPosition =
            boss.GetPosition();

        Vector3 playerToBoss =
            Vector3Subtract(
                bossPosition,
                playerPosition
            );

        // Boss uzun olduğu için yalnızca yatay mesafeyi
        // hesaba katıyoruz.
        playerToBoss.y = 0.0f;

        const float distanceToBoss =
            Vector3Length(
                playerToBoss
            );

        if (
            distanceToBoss <=
            playerAttackRange + 1.2f
            )
        {
            Vector3 directionToBoss{
                0.0f,
                0.0f,
                0.0f
            };

            if (distanceToBoss > 0.001f)
            {
                directionToBoss =
                    Vector3Normalize(
                        playerToBoss
                    );
            }

            Vector3 playerFacingDirection =
                player.GetFacingDirection();

            playerFacingDirection.y = 0.0f;

            if (
                Vector3Length(
                    playerFacingDirection
                ) > 0.001f
                )
            {
                playerFacingDirection =
                    Vector3Normalize(
                        playerFacingDirection
                    );
            }

            const float facingAmount =
                Vector3DotProduct(
                    playerFacingDirection,
                    directionToBoss
                );

            if (facingAmount > 0.25f)
            {
                boss.TakeDamage(
                    playerAttackDamage
                );

                boss.ApplyKnockback(
                    directionToBoss,
                    playerKnockbackForce
                );

                cameraShakeRequested = true;
            }
        }
    }

    playerAttackWasActive =
        playerAttackIsActive;
}

void CombatSystem::HandleEnemyAttacks(
    float deltaTime,
    Player& player,
    std::vector<Enemy>& enemies
)
{
    if (
        enemyAttackCooldownTimers.size() !=
        enemies.size()
        )
    {
        enemyAttackCooldownTimers.resize(
            enemies.size(),
            0.0f
        );
    }

    for (
        std::size_t index = 0;
        index < enemies.size();
        ++index
        )
    {
        float& cooldownTimer =
            enemyAttackCooldownTimers[index];

        if (cooldownTimer > 0.0f)
        {
            cooldownTimer -= deltaTime;

            if (cooldownTimer < 0.0f)
            {
                cooldownTimer = 0.0f;
            }
        }
    }

    if (!player.IsAlive())
    {
        return;
    }

    const Vector3 playerPosition =
        player.GetPosition();

    for (
        std::size_t index = 0;
        index < enemies.size();
        ++index
        )
    {
        Enemy& enemy =
            enemies[index];

        if (!enemy.IsAlive())
        {
            continue;
        }

        const Vector3 enemyPosition =
            enemy.GetPosition();

        const Vector3 enemyToPlayer =
            Vector3Subtract(
                playerPosition,
                enemyPosition
            );

        const float distanceToPlayer =
            Vector3Length(
                enemyToPlayer
            );

        if (
            distanceToPlayer <= enemyAttackRange &&
            enemyAttackCooldownTimers[index] <= 0.0f
            )
        {
            player.TakeDamage(
                enemyAttackDamage
            );

            enemyAttackCooldownTimers[index] =
                enemyAttackCooldown;

            cameraShakeRequested = true;

            if (!player.IsAlive())
            {
                break;
            }
        }
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

void CombatSystem::IncreasePlayerAttackDamage(
    float amount
)
{
    if (amount <= 0.0f)
    {
        return;
    }

    playerAttackDamage += amount;
}

float CombatSystem::GetPlayerAttackDamage() const
{
    return playerAttackDamage;
}