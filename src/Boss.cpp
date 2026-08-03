#include "Boss.h"

#include "Player.h"

#include <cmath>

#include <raymath.h>
#include <rlgl.h>

namespace
{
    constexpr float TITAN_VISUAL_SCALE = 0.60f;

    void DrawTitanLimb(
        Vector3 start,
        Vector3 end,
        float startRadius,
        float endRadius,
        Color color
    )
    {
        DrawCylinderEx(
            start,
            end,
            startRadius,
            endRadius,
            9,
            color
        );
        DrawSphere(start, startRadius * 1.02f, color);
        DrawSphere(end, endRadius * 1.05f, color);
    }
}

Boss::Boss(Vector3 startPosition)
    : position(startPosition),
    knockbackVelocity{ 0.0f, 0.0f, 0.0f },
    facingDirection{ 0.0f, 0.0f, 1.0f },
    attackTarget(startPosition),
    normalMoveSpeed(1.65f),
    rageMoveSpeed(2.65f),
    health(850.0f),
    maxHealth(850.0f),
    attackRange(3.0f),
    attackDamage(28),
    attackCooldown(1.45f),
    attackCooldownTimer(0.45f),
    hitFlashTimer(0.0f),
    hitFlashDuration(0.18f),
    animationTime(0.0f),
    alive(true),
    rageModeActive(false),
    attackHitRequested(false),
    currentAttack(AttackType::None),
    attackWindupTimer(0.0f),
    meleeWindupDuration(0.72f),
    rageMeleeWindupDuration(0.46f),
    meteorWindupDuration(1.35f),
    meteorRange(2.5f),
    meteorDamage(24),
    rageSlamWindupDuration(1.15f),
    rageSlamRange(6.0f),
    rageSlamDamage(40),
    attacksSinceLastSlam(0)
{
}

void Boss::Update(float deltaTime, Player& player)
{
    attackHitRequested = false;

    if (!alive)
    {
        return;
    }

    animationTime += deltaTime;
    UpdateHitFlash(deltaTime);
    UpdateKnockback(deltaTime);

    rageModeActive = health <= maxHealth * 0.5f;

    UpdateMovement(deltaTime, player.GetPosition());
    UpdateAttack(deltaTime, player);
}

void Boss::UpdateHitFlash(float deltaTime)
{
    if (hitFlashTimer > 0.0f)
    {
        hitFlashTimer -= deltaTime;

        if (hitFlashTimer < 0.0f)
        {
            hitFlashTimer = 0.0f;
        }
    }
}

void Boss::UpdateKnockback(float deltaTime)
{
    position.x += knockbackVelocity.x * deltaTime;
    position.z += knockbackVelocity.z * deltaTime;

    knockbackVelocity = Vector3Scale(knockbackVelocity, 0.88f);

    if (Vector3Length(knockbackVelocity) < 0.05f)
    {
        knockbackVelocity = Vector3{ 0.0f, 0.0f, 0.0f };
    }
}

void Boss::UpdateMovement(float deltaTime, Vector3 playerPosition)
{
    Vector3 toPlayer = Vector3Subtract(playerPosition, position);
    toPlayer.y = 0.0f;

    const float distance = Vector3Length(toPlayer);

    if (distance > 0.001f)
    {
        facingDirection = Vector3Normalize(toPlayer);
    }

    if (
        currentAttack != AttackType::None ||
        distance <= attackRange ||
        distance <= 0.001f
        )
    {
        return;
    }

    const float speed = rageModeActive
        ? rageMoveSpeed
        : normalMoveSpeed;

    position.x += facingDirection.x * speed * deltaTime;
    position.z += facingDirection.z * speed * deltaTime;
}

void Boss::UpdateAttack(float deltaTime, Player& player)
{
    if (attackCooldownTimer > 0.0f)
    {
        attackCooldownTimer -= deltaTime;

        if (attackCooldownTimer < 0.0f)
        {
            attackCooldownTimer = 0.0f;
        }
    }

    if (!player.IsAlive())
    {
        currentAttack = AttackType::None;
        attackWindupTimer = 0.0f;
        return;
    }

    if (currentAttack != AttackType::None)
    {
        attackWindupTimer -= deltaTime;

        if (attackWindupTimer > 0.0f)
        {
            return;
        }

        attackWindupTimer = 0.0f;

        if (currentAttack == AttackType::Melee)
        {
            ResolveMeleeAttack(player);
        }
        else if (currentAttack == AttackType::MeteorStrike)
        {
            ResolveMeteorStrike(player);
        }
        else if (currentAttack == AttackType::RageSlam)
        {
            ResolveRageSlamAttack(player);
        }

        currentAttack = AttackType::None;
        attackCooldownTimer = rageModeActive
            ? attackCooldown * 0.68f
            : attackCooldown;
        return;
    }

    if (attackCooldownTimer > 0.0f)
    {
        return;
    }

    Vector3 toPlayer = Vector3Subtract(player.GetPosition(), position);
    toPlayer.y = 0.0f;
    const float distance = Vector3Length(toPlayer);

    if (
        rageModeActive &&
        attacksSinceLastSlam >= 2 &&
        distance <= rageSlamRange
        )
    {
        StartRageSlamAttack();
    }
    else if (distance <= attackRange)
    {
        StartMeleeAttack();
    }
    else
    {
        StartMeteorStrike(player.GetPosition());
    }
}

void Boss::StartMeleeAttack()
{
    currentAttack = AttackType::Melee;
    attackWindupTimer = rageModeActive
        ? rageMeleeWindupDuration
        : meleeWindupDuration;
}

void Boss::StartMeteorStrike(Vector3 playerPosition)
{
    currentAttack = AttackType::MeteorStrike;
    attackTarget = playerPosition;
    attackTarget.y = 0.0f;
    attackWindupTimer = rageModeActive
        ? meteorWindupDuration * 0.78f
        : meteorWindupDuration;
}

void Boss::StartRageSlamAttack()
{
    currentAttack = AttackType::RageSlam;
    attackWindupTimer = rageSlamWindupDuration;
}

void Boss::ResolveMeleeAttack(Player& player)
{
    Vector3 toPlayer = Vector3Subtract(player.GetPosition(), position);
    toPlayer.y = 0.0f;

    if (Vector3Length(toPlayer) <= attackRange)
    {
        player.TakeDamage(attackDamage);
        attackHitRequested = true;
    }

    attacksSinceLastSlam++;
}

void Boss::ResolveMeteorStrike(Player& player)
{
    Vector3 toTarget = Vector3Subtract(player.GetPosition(), attackTarget);
    toTarget.y = 0.0f;

    if (Vector3Length(toTarget) <= meteorRange)
    {
        player.TakeDamage(meteorDamage);
        attackHitRequested = true;
    }

    attacksSinceLastSlam++;
}

void Boss::ResolveRageSlamAttack(Player& player)
{
    Vector3 toPlayer = Vector3Subtract(player.GetPosition(), position);
    toPlayer.y = 0.0f;

    if (Vector3Length(toPlayer) <= rageSlamRange)
    {
        player.TakeDamage(rageSlamDamage);
        attackHitRequested = true;
    }

    attacksSinceLastSlam = 0;
}

void Boss::DrawAttackWarning() const
{
    if (currentAttack == AttackType::None)
    {
        return;
    }

    if (currentAttack == AttackType::MeteorStrike)
    {
        const float progress = 1.0f -
            Clamp(
                attackWindupTimer / meteorWindupDuration,
                0.0f,
                1.0f
            );

        DrawCircle3D(
            Vector3{ attackTarget.x, 0.04f, attackTarget.z },
            meteorRange,
            Vector3{ 1.0f, 0.0f, 0.0f },
            90.0f,
            Fade(RED, 0.28f + progress * 0.30f)
        );

        DrawCircle3D(
            Vector3{ attackTarget.x, 0.05f, attackTarget.z },
            meteorRange * (1.0f - progress * 0.72f),
            Vector3{ 1.0f, 0.0f, 0.0f },
            90.0f,
            Fade(GOLD, 0.72f)
        );

        const float meteorHeight = 9.5f - progress * 8.7f;
        const float meteorRadius = 0.34f + progress * 0.12f;
        const Vector3 meteorPosition{
            attackTarget.x,
            meteorHeight,
            attackTarget.z
        };

        // The meteor stays small enough to preserve the player's view.
        // A short fire trail makes its direction readable without a giant orb.
        DrawSphere(meteorPosition, meteorRadius, Color{ 74, 48, 42, 255 });
        DrawSphere(
            Vector3{ meteorPosition.x, meteorPosition.y + 0.42f, meteorPosition.z },
            meteorRadius * 0.72f,
            Color{ 255, 92, 22, 220 }
        );
        DrawSphere(
            Vector3{ meteorPosition.x, meteorPosition.y + 0.78f, meteorPosition.z },
            meteorRadius * 0.42f,
            Color{ 255, 190, 48, 150 }
        );
        return;
    }

    const float range = currentAttack == AttackType::RageSlam
        ? rageSlamRange
        : attackRange;

    const Color warningColor = currentAttack == AttackType::RageSlam
        ? RED
        : ORANGE;

    DrawCircle3D(
        Vector3{ position.x, 0.04f, position.z },
        range,
        Vector3{ 1.0f, 0.0f, 0.0f },
        90.0f,
        Fade(warningColor, 0.48f)
    );
}

void Boss::Draw() const
{
    if (!alive)
    {
        return;
    }

    DrawAttackWarning();

    const Color stoneColor = hitFlashTimer > 0.0f
        ? Color{ 255, 236, 178, 255 }
        : rageModeActive
        ? Color{ 78, 52, 48, 255 }
        : Color{ 62, 60, 68, 255 };

    const Color darkStone = rageModeActive
        ? Color{ 48, 30, 29, 255 }
        : Color{ 38, 38, 46, 255 };

    const Color energyColor = rageModeActive
        ? Color{ 255, 63, 16, 255 }
        : Color{ 171, 64, 255, 255 };

    const float walkBob =
        currentAttack == AttackType::None
        ? sinf(animationTime * 4.0f) * 0.08f
        : 0.0f;

    const float facingAngle =
        atan2f(facingDirection.x, facingDirection.z) * RAD2DEG;

    rlPushMatrix();
    // The simulation position remains unchanged. Only the rendered Titan is
    // scaled around a ground-aligned origin so it is imposing but never fills
    // the entire camera view.
    rlTranslatef(position.x, position.y - 1.08f + walkBob, position.z);
    rlRotatef(facingAngle, 0.0f, 1.0f, 0.0f);
    rlScalef(TITAN_VISUAL_SCALE, TITAN_VISUAL_SCALE, TITAN_VISUAL_SCALE);

    // Broad rock torso and layered armor plates.
    DrawCube(Vector3{ 0.0f, 0.0f, 0.0f }, 3.5f, 4.1f, 2.5f, darkStone);
    DrawCube(Vector3{ 0.0f, 0.55f, 0.68f }, 3.0f, 2.2f, 0.55f, stoneColor);
    DrawSphere(Vector3{ 0.0f, 1.25f, 0.25f }, 1.55f, stoneColor);

    // Glowing Aether heart and cracks.
    const float corePulse = 0.42f + 0.08f * sinf(animationTime * 5.0f);
    DrawSphere(Vector3{ 0.0f, 0.75f, 1.38f }, corePulse, energyColor);
    DrawSphere(Vector3{ 0.0f, 0.75f, 1.38f }, corePulse * 1.8f, Fade(energyColor, 0.16f));
    DrawCube(Vector3{ 0.0f, -0.05f, 1.34f }, 0.13f, 1.05f, 0.10f, energyColor);
    DrawCube(Vector3{ -0.46f, 0.25f, 1.34f }, 0.82f, 0.11f, 0.10f, energyColor);
    DrawCube(Vector3{ 0.46f, 0.25f, 1.34f }, 0.82f, 0.11f, 0.10f, energyColor);

    // Shoulders and heavy arms give the Titan a distinct silhouette.
    DrawSphere(Vector3{ -2.05f, 1.15f, 0.0f }, 1.05f, stoneColor);
    DrawSphere(Vector3{  2.05f, 1.15f, 0.0f }, 1.05f, stoneColor);

    const float armLift =
        currentAttack == AttackType::RageSlam
        ? 1.25f
        : currentAttack == AttackType::MeteorStrike
        ? 0.75f
        : 0.0f;

    DrawTitanLimb(
        Vector3{ -2.05f, 0.95f, 0.0f },
        Vector3{ -2.42f, -1.05f + armLift, 0.28f },
        0.78f, 0.62f, stoneColor
    );
    DrawTitanLimb(
        Vector3{ 2.05f, 0.95f, 0.0f },
        Vector3{ 2.42f, -1.05f + armLift, 0.28f },
        0.78f, 0.62f, stoneColor
    );

    DrawSphere(Vector3{ -2.48f, -1.32f + armLift, 0.40f }, 0.76f, darkStone);
    DrawSphere(Vector3{  2.48f, -1.32f + armLift, 0.40f }, 0.76f, darkStone);

    // Head, jaw, horns and glowing eyes.
    DrawCube(Vector3{ 0.0f, 2.65f, 0.08f }, 2.15f, 1.55f, 1.75f, stoneColor);
    DrawCube(Vector3{ 0.0f, 2.28f, 0.98f }, 1.55f, 0.58f, 0.42f, darkStone);

    DrawCylinderEx(
        Vector3{ -0.85f, 3.15f, 0.0f },
        Vector3{ -1.65f, 4.05f, -0.05f },
        0.30f, 0.03f, 7, darkStone
    );
    DrawCylinderEx(
        Vector3{ 0.85f, 3.15f, 0.0f },
        Vector3{ 1.65f, 4.05f, -0.05f },
        0.30f, 0.03f, 7, darkStone
    );

    DrawSphere(Vector3{ -0.48f, 2.82f, 0.98f }, 0.18f, energyColor);
    DrawSphere(Vector3{  0.48f, 2.82f, 0.98f }, 0.18f, energyColor);

    // Split stone legs and wide feet.
    DrawTitanLimb(
        Vector3{ -0.95f, -1.85f, 0.0f },
        Vector3{ -1.02f, -3.15f, 0.12f },
        0.76f, 0.62f, darkStone
    );
    DrawTitanLimb(
        Vector3{ 0.95f, -1.85f, 0.0f },
        Vector3{ 1.02f, -3.15f, 0.12f },
        0.76f, 0.62f, darkStone
    );
    DrawCube(Vector3{ -1.02f, -3.38f, 0.45f }, 1.42f, 0.62f, 1.85f, stoneColor);
    DrawCube(Vector3{  1.02f, -3.38f, 0.45f }, 1.42f, 0.62f, 1.85f, stoneColor);

    rlPopMatrix();
}

void Boss::TakeDamage(float damage)
{
    if (!alive || damage <= 0.0f)
    {
        return;
    }

    health -= damage;
    hitFlashTimer = hitFlashDuration;

    if (health <= 0.0f)
    {
        health = 0.0f;
        alive = false;
        currentAttack = AttackType::None;
        attackWindupTimer = 0.0f;
    }
}

void Boss::ApplyKnockback(Vector3 direction, float force)
{
    if (
        !alive ||
        force <= 0.0f ||
        Vector3Length(direction) <= 0.001f
        )
    {
        return;
    }

    knockbackVelocity = Vector3Scale(
        Vector3Normalize(direction),
        force * 0.16f
    );
}

bool Boss::IsAlive() const
{
    return alive;
}

Vector3 Boss::GetPosition() const
{
    return position;
}

float Boss::GetHealth() const
{
    return health;
}

float Boss::GetMaxHealth() const
{
    return maxHealth;
}

bool Boss::IsRageModeActive() const
{
    return rageModeActive;
}

bool Boss::ConsumeAttackHitRequest()
{
    if (!attackHitRequested)
    {
        return false;
    }

    attackHitRequested = false;
    return true;
}
