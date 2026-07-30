#include "Enemy.h"

#include <raymath.h>

Enemy::Enemy(Vector3 startPosition)
    : position(startPosition),
    homePosition(startPosition),
    knockbackVelocity{ 0.0f, 0.0f, 0.0f },
    facingDirection{ 0.0f, 0.0f, 1.0f },
    moveSpeed(2.5f),
    returnSpeed(1.8f),
    health(100.0f),
    maxHealth(100.0f),
    hitFlashTimer(0.0f),
    hitFlashDuration(0.25f),
    detectionRange(8.0f),
    loseTargetRange(13.0f),
    forgetDuration(2.5f),
    forgetTimer(0.0f),
    homeStopDistance(0.25f),
    alerted(false),
    alive(true),
    scoreValue(100),
    scoreGiven(false)
{
}

void Enemy::Update(
    float deltaTime,
    Vector3 playerPosition,
    Vector3 separationForce
)
{
    if (!alive)
    {
        return;
    }

    // =====================================================
    // HIT FLASH TIMER
    // =====================================================

    if (hitFlashTimer > 0.0f)
    {
        hitFlashTimer -= deltaTime;

        if (hitFlashTimer < 0.0f)
        {
            hitFlashTimer = 0.0f;
        }
    }

    // =====================================================
    // KNOCKBACK
    // =====================================================

    position.x +=
        knockbackVelocity.x *
        deltaTime;

    position.z +=
        knockbackVelocity.z *
        deltaTime;

    knockbackVelocity =
        Vector3Scale(
            knockbackVelocity,
            0.88f
        );

    if (
        Vector3Length(
            knockbackVelocity
        ) < 0.05f
        )
    {
        knockbackVelocity =
        {
            0.0f,
            0.0f,
            0.0f
        };
    }

    // Y ekseninde hareket etmiyoruz.
    playerPosition.y = position.y;

    Vector3 toPlayer =
        Vector3Subtract(
            playerPosition,
            position
        );

    const float playerDistance =
        Vector3Length(
            toPlayer
        );

    // =====================================================
    // PLAYER DETECTION
    // =====================================================

    if (!alerted)
    {
        if (playerDistance <= detectionRange)
        {
            alerted = true;

            forgetTimer =
                forgetDuration;
        }
    }
    else
    {
        if (playerDistance <= loseTargetRange)
        {
            forgetTimer =
                forgetDuration;
        }
        else
        {
            forgetTimer -=
                deltaTime;

            if (forgetTimer <= 0.0f)
            {
                forgetTimer = 0.0f;

                alerted = false;
            }
        }
    }

    Vector3 movementDirection =
    {
        0.0f,
        0.0f,
        0.0f
    };

    float selectedMoveSpeed =
        moveSpeed;

    // =====================================================
    // CHASE PLAYER
    // =====================================================

    if (alerted)
    {
        if (playerDistance > 0.1f)
        {
            movementDirection =
                Vector3Normalize(
                    toPlayer
                );
        }

        selectedMoveSpeed =
            moveSpeed;
    }
    else
    {
        // =================================================
        // RETURN TO HOME POSITION
        // =================================================

        Vector3 toHome =
            Vector3Subtract(
                homePosition,
                position
            );

        toHome.y = 0.0f;

        const float homeDistance =
            Vector3Length(
                toHome
            );

        if (homeDistance > homeStopDistance)
        {
            movementDirection =
                Vector3Normalize(
                    toHome
                );

            selectedMoveSpeed =
                returnSpeed;
        }
    }

    // =====================================================
    // ENEMY SEPARATION
    // =====================================================

    movementDirection =
        Vector3Add(
            movementDirection,
            separationForce
        );

    movementDirection.y = 0.0f;

    if (
        Vector3Length(
            movementDirection
        ) > 0.01f
        )
    {
        movementDirection =
            Vector3Normalize(
                movementDirection
            );

        facingDirection =
            movementDirection;

        position.x +=
            movementDirection.x *
            selectedMoveSpeed *
            deltaTime;

        position.z +=
            movementDirection.z *
            selectedMoveSpeed *
            deltaTime;
    }

    // Düşmanların zeminden yukarı veya aşağı kaymasını önler.
    position.y =
        homePosition.y;
}

void Enemy::Draw() const
{
    if (!alive)
    {
        return;
    }

    const Color bodyColor =
        hitFlashTimer > 0.0f
        ? Color{
            255,
            255,
            180,
            255
    }
        : alerted
        ? RED
        : Color{
            175,
            60,
            60,
            255
    };

    DrawCube(
        position,
        1.0f,
        2.0f,
        1.0f,
        bodyColor
    );

    DrawCubeWires(
        position,
        1.0f,
        2.0f,
        1.0f,
        alerted
        ? MAROON
        : DARKGRAY
    );

    // =====================================================
    // ALERT INDICATOR
    // =====================================================

    if (alerted)
    {
        const Vector3 alertPosition =
        {
            position.x,
            position.y + 1.7f,
            position.z
        };

        DrawSphere(
            alertPosition,
            0.12f,
            YELLOW
        );
    }
}

void Enemy::TakeDamage(float damage)
{
    if (!alive)
    {
        return;
    }

    health -= damage;

    hitFlashTimer =
        hitFlashDuration;

    // Düşmana vurulduğunda oyuncuyu görmese bile alarma geçer.
    alerted = true;

    forgetTimer =
        forgetDuration;

    if (health < 0.0f)
    {
        health = 0.0f;
    }

    if (health <= 0.0f)
    {
        alive = false;
    }
}

void Enemy::ApplyKnockback(
    Vector3 direction,
    float force
)
{
    if (
        !alive ||
        Vector3Length(direction) <= 0.0f
        )
    {
        return;
    }

    knockbackVelocity =
        Vector3Scale(
            Vector3Normalize(
                direction
            ),
            force
        );
}

bool Enemy::IsAlive() const
{
    return alive;
}

Vector3 Enemy::GetPosition() const
{
    return position;
}

float Enemy::GetHealth() const
{
    return health;
}

float Enemy::GetMaxHealth() const
{
    return maxHealth;
}

bool Enemy::IsAlerted() const
{
    return alerted;
}

// ==========================
// SCORE SYSTEM
// ==========================

int Enemy::GetScoreValue() const
{
    return scoreValue;
}

bool Enemy::ScoreAlreadyGiven() const
{
    return scoreGiven;
}

void Enemy::MarkScoreGiven()
{
    scoreGiven = true;
}