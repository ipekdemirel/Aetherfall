#include "Enemy.h"

#include <cmath>

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
    visualTime(0.0f),
    deathEffectTimer(0.0f),
    deathEffectDuration(0.72f),
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
    Vector3 separationForce,
    bool forceChaseTarget
)
{
    visualTime += deltaTime;

    if (!alive)
    {
        if (deathEffectTimer > 0.0f)
        {
            deathEffectTimer -= deltaTime;

            if (deathEffectTimer < 0.0f)
            {
                deathEffectTimer = 0.0f;
            }
        }

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

    if (forceChaseTarget)
    {
        alerted = true;
        forgetTimer = forgetDuration;
    }
    else if (!alerted)
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
        if (deathEffectTimer <= 0.0f)
        {
            return;
        }

        const float effectProgress =
            1.0f -
            deathEffectTimer /
            deathEffectDuration;

        const unsigned char effectAlpha =
            static_cast<unsigned char>(
                220.0f *
                (1.0f - effectProgress)
            );

        DrawCircle3D(
            Vector3{
                position.x,
                0.035f,
                position.z
            },
            0.65f + effectProgress * 1.45f,
            Vector3{ 1.0f, 0.0f, 0.0f },
            90.0f,
            Color{ 126, 70, 255, effectAlpha }
        );

        for (int particle = 0;
            particle < 9;
            ++particle)
        {
            const float angle =
                particle *
                (2.0f * PI / 9.0f) +
                visualTime * 2.0f;

            const float distance =
                0.28f +
                effectProgress * 1.45f;

            DrawCube(
                Vector3{
                    position.x +
                        cosf(angle) * distance,
                    0.45f +
                        effectProgress *
                        (1.1f + (particle % 3) * 0.25f),
                    position.z +
                        sinf(angle) * distance
                },
                0.15f,
                0.15f,
                0.15f,
                Color{
                    static_cast<unsigned char>(
                        132 + (particle % 3) * 24
                    ),
                    82,
                    255,
                    effectAlpha
                }
            );
        }

        return;
    }

    const float hover =
        sinf(
            visualTime * 3.2f +
            position.x * 0.37f +
            position.z * 0.21f
        ) * 0.08f;

    const Color robeColor =
        hitFlashTimer > 0.0f
        ? Color{ 255, 238, 180, 255 }
        : alerted
        ? Color{ 92, 35, 124, 255 }
        : Color{ 48, 42, 72, 255 };

    const Color armorColor =
        hitFlashTimer > 0.0f
        ? Color{ 255, 248, 210, 255 }
        : alerted
        ? Color{ 121, 52, 155, 255 }
        : Color{ 70, 64, 94, 255 };

    const Color energyColor =
        alerted
        ? Color{ 255, 69, 126, 255 }
        : Color{ 169, 89, 255, 255 };

    DrawCircle3D(
        Vector3{
            position.x,
            0.025f,
            position.z
        },
        0.72f,
        Vector3{ 1.0f, 0.0f, 0.0f },
        90.0f,
        Fade(BLACK, 0.52f)
    );

    const Vector3 robePosition{
        position.x,
        position.y - 0.20f + hover,
        position.z
    };

    DrawCylinder(
        robePosition,
        0.30f,
        0.68f,
        1.42f,
        12,
        robeColor
    );

    DrawCylinderWires(
        robePosition,
        0.30f,
        0.68f,
        1.42f,
        12,
        Color{ 28, 23, 42, 255 }
    );

    const Vector3 chestPosition{
        position.x,
        position.y + 0.42f + hover,
        position.z
    };

    DrawCylinder(
        chestPosition,
        0.43f,
        0.34f,
        0.72f,
        10,
        armorColor
    );

    const Vector3 headPosition{
        position.x,
        position.y + 1.02f + hover,
        position.z
    };

    DrawSphere(
        headPosition,
        0.36f,
        Color{ 35, 30, 50, 255 }
    );

    Vector3 flatFacing =
        facingDirection;

    flatFacing.y = 0.0f;

    if (Vector3Length(flatFacing) < 0.01f)
    {
        flatFacing =
            Vector3{ 0.0f, 0.0f, 1.0f };
    }
    else
    {
        flatFacing =
            Vector3Normalize(flatFacing);
    }

    const Vector3 rightDirection{
        -flatFacing.z,
        0.0f,
        flatFacing.x
    };

    const Vector3 faceCenter =
        Vector3Add(
            headPosition,
            Vector3Scale(flatFacing, 0.31f)
        );

    for (int side = -1; side <= 1; side += 2)
    {
        DrawSphere(
            Vector3Add(
                faceCenter,
                Vector3Scale(
                    rightDirection,
                    side * 0.11f
                )
            ),
            0.055f,
            energyColor
        );
    }

    const Vector3 leftHornBase =
        Vector3Add(
            headPosition,
            Vector3Scale(rightDirection, -0.22f)
        );

    const Vector3 rightHornBase =
        Vector3Add(
            headPosition,
            Vector3Scale(rightDirection, 0.22f)
        );

    DrawCylinderEx(
        leftHornBase,
        Vector3Add(
            leftHornBase,
            Vector3{
                rightDirection.x * -0.22f,
                0.42f,
                rightDirection.z * -0.22f
            }
        ),
        0.10f,
        0.02f,
        7,
        Color{ 104, 95, 126, 255 }
    );

    DrawCylinderEx(
        rightHornBase,
        Vector3Add(
            rightHornBase,
            Vector3{
                rightDirection.x * 0.22f,
                0.42f,
                rightDirection.z * 0.22f
            }
        ),
        0.10f,
        0.02f,
        7,
        Color{ 104, 95, 126, 255 }
    );

    const Vector3 weaponHand =
        Vector3Add(
            chestPosition,
            Vector3Add(
                Vector3Scale(rightDirection, 0.53f),
                Vector3Scale(flatFacing, 0.12f)
            )
        );

    DrawCylinderEx(
        Vector3{
            weaponHand.x,
            weaponHand.y - 0.55f,
            weaponHand.z
        },
        Vector3{
            weaponHand.x,
            weaponHand.y + 0.80f,
            weaponHand.z
        },
        0.045f,
        0.045f,
        6,
        Color{ 45, 39, 55, 255 }
    );

    DrawCylinderEx(
        Vector3{
            weaponHand.x,
            weaponHand.y + 0.56f,
            weaponHand.z
        },
        Vector3Add(
            Vector3{
                weaponHand.x,
                weaponHand.y + 0.78f,
                weaponHand.z
            },
            Vector3Scale(rightDirection, -0.34f)
        ),
        0.13f,
        0.015f,
        7,
        energyColor
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
            0.09f +
                0.03f *
                sinf(visualTime * 8.0f),
            energyColor
        );

        DrawCircle3D(
            Vector3{
                position.x,
                0.04f,
                position.z
            },
            0.88f +
                0.06f *
                sinf(visualTime * 5.0f),
            Vector3{ 1.0f, 0.0f, 0.0f },
            90.0f,
            Fade(energyColor, 0.55f)
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
        deathEffectTimer =
            deathEffectDuration;
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
