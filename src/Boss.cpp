#include "Boss.h"

#include "Player.h"

#include <raymath.h>

Boss::Boss(Vector3 startPosition)
    :
    position(startPosition),
    knockbackVelocity{ 0.0f, 0.0f, 0.0f },

    normalMoveSpeed(1.5f),
    rageMoveSpeed(2.4f),

    health(700.0f),
    maxHealth(700.0f),

    attackRange(2.4f),
    attackDamage(30),

    attackCooldown(1.5f),
    attackCooldownTimer(0.0f),

    hitFlashTimer(0.0f),
    hitFlashDuration(0.20f),

    alive(true),
    rageModeActive(false),

    attackHitRequested(false),

    currentAttack(AttackType::None),
    attackWindupTimer(0.0f),

    meleeWindupDuration(0.70f),
    rageMeleeWindupDuration(0.45f),

    rageSlamWindupDuration(1.15f),
    rageSlamRange(5.5f),
    rageSlamDamage(40),

    attacksSinceLastSlam(0)
{
}

void Boss::Update(
    float deltaTime,
    Player& player
)
{
    attackHitRequested = false;

    if (!alive)
    {
        return;
    }

    UpdateHitFlash(deltaTime);

    UpdateKnockback(deltaTime);

    rageModeActive =
        health <= maxHealth * 0.5f;

    UpdateMovement(
        deltaTime,
        player.GetPosition()
    );

    UpdateAttack(
        deltaTime,
        player
    );
}

void Boss::UpdateHitFlash(float deltaTime)
{
    if (hitFlashTimer <= 0.0f)
    {
        return;
    }

    hitFlashTimer -= deltaTime;

    if (hitFlashTimer < 0.0f)
    {
        hitFlashTimer = 0.0f;
    }
}

void Boss::UpdateKnockback(float deltaTime)
{
    position.x +=
        knockbackVelocity.x * deltaTime;

    position.z +=
        knockbackVelocity.z * deltaTime;

    knockbackVelocity =
        Vector3Scale(
            knockbackVelocity,
            0.88f
        );

    if (
        Vector3Length(knockbackVelocity) <
        0.05f
        )
    {
        knockbackVelocity = {
            0.0f,
            0.0f,
            0.0f
        };
    }
}

void Boss::UpdateMovement(
    float deltaTime,
    Vector3 playerPosition
)
{
    // Boss bir saldırı hazırlıyorsa hareket etmez.
    // Böylece oyuncu saldırıdan kaçma fırsatı bulur.

    if (currentAttack != AttackType::None)
    {
        return;
    }

    Vector3 bossToPlayer =
        Vector3Subtract(
            playerPosition,
            position
        );

    bossToPlayer.y = 0.0f;

    const float distanceToPlayer =
        Vector3Length(bossToPlayer);

    if (
        distanceToPlayer <= attackRange ||
        distanceToPlayer <= 0.001f
        )
    {
        return;
    }

    const Vector3 movementDirection =
        Vector3Normalize(
            bossToPlayer
        );

    const float currentMoveSpeed =
        rageModeActive
        ? rageMoveSpeed
        : normalMoveSpeed;

    position.x +=
        movementDirection.x *
        currentMoveSpeed *
        deltaTime;

    position.z +=
        movementDirection.z *
        currentMoveSpeed *
        deltaTime;
}

void Boss::UpdateAttack(
    float deltaTime,
    Player& player
)
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
        currentAttack =
            AttackType::None;

        attackWindupTimer = 0.0f;

        return;
    }

    // Boss şu anda saldırı hazırlıyorsa
    // hazırlık süresini azalt.

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
        else if (currentAttack == AttackType::RageSlam)
        {
            ResolveRageSlamAttack(player);
        }

        currentAttack =
            AttackType::None;

        attackCooldownTimer =
            rageModeActive
            ? attackCooldown * 0.70f
            : attackCooldown;

        return;
    }

    if (attackCooldownTimer > 0.0f)
    {
        return;
    }

    Vector3 bossToPlayer =
        Vector3Subtract(
            player.GetPosition(),
            position
        );

    bossToPlayer.y = 0.0f;

    const float distanceToPlayer =
        Vector3Length(bossToPlayer);

    // Rage modunda Boss her iki yakın saldırıdan
    // sonra bir alan saldırısı hazırlayabilir.

    if (
        rageModeActive &&
        attacksSinceLastSlam >= 2 &&
        distanceToPlayer <= rageSlamRange
        )
    {
        StartRageSlamAttack();
        return;
    }

    if (distanceToPlayer <= attackRange)
    {
        StartMeleeAttack();
    }
}

void Boss::StartMeleeAttack()
{
    currentAttack =
        AttackType::Melee;

    attackWindupTimer =
        rageModeActive
        ? rageMeleeWindupDuration
        : meleeWindupDuration;
}

void Boss::StartRageSlamAttack()
{
    currentAttack =
        AttackType::RageSlam;

    attackWindupTimer =
        rageSlamWindupDuration;
}

void Boss::ResolveMeleeAttack(Player& player)
{
    Vector3 bossToPlayer =
        Vector3Subtract(
            player.GetPosition(),
            position
        );

    bossToPlayer.y = 0.0f;

    const float distanceToPlayer =
        Vector3Length(bossToPlayer);

    // Oyuncu uyarıyı görüp menzilden çıktıysa
    // saldırı boşa gider.

    if (distanceToPlayer <= attackRange)
    {
        player.TakeDamage(
            attackDamage
        );

        attackHitRequested = true;
    }

    attacksSinceLastSlam++;
}

void Boss::ResolveRageSlamAttack(Player& player)
{
    Vector3 bossToPlayer =
        Vector3Subtract(
            player.GetPosition(),
            position
        );

    bossToPlayer.y = 0.0f;

    const float distanceToPlayer =
        Vector3Length(bossToPlayer);

    // Oyuncu kırmızı alanın dışına çıkarsa
    // alan saldırısından hasar almaz.

    if (distanceToPlayer <= rageSlamRange)
    {
        player.TakeDamage(
            rageSlamDamage
        );

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

    const Vector3 warningPosition{
        position.x,
        position.y - 2.25f,
        position.z
    };

    if (currentAttack == AttackType::Melee)
    {
        DrawCircle3D(
            warningPosition,
            attackRange,
            Vector3{ 1.0f, 0.0f, 0.0f },
            90.0f,
            Fade(ORANGE, 0.55f)
        );
    }
    else if (currentAttack == AttackType::RageSlam)
    {
        DrawCircle3D(
            warningPosition,
            rageSlamRange,
            Vector3{ 1.0f, 0.0f, 0.0f },
            90.0f,
            Fade(RED, 0.45f)
        );
    }
}

void Boss::Draw() const
{
    if (!alive)
    {
        return;
    }

    DrawAttackWarning();

    Color bodyColor;

    if (hitFlashTimer > 0.0f)
    {
        bodyColor =
            Color{
                255,
                255,
                190,
                255
        };
    }
    else if (currentAttack == AttackType::RageSlam)
    {
        bodyColor =
            Color{
                255,
                70,
                20,
                255
        };
    }
    else if (currentAttack == AttackType::Melee)
    {
        bodyColor =
            Color{
                230,
                110,
                25,
                255
        };
    }
    else if (rageModeActive)
    {
        bodyColor =
            Color{
                180,
                20,
                30,
                255
        };
    }
    else
    {
        bodyColor =
            Color{
                95,
                25,
                125,
                255
        };
    }

    // Boss ana gövdesi

    DrawCube(
        position,
        3.2f,
        4.5f,
        3.2f,
        bodyColor
    );

    DrawCubeWires(
        position,
        3.2f,
        4.5f,
        3.2f,
        BLACK
    );

    // Boss başı

    const Vector3 headPosition{
        position.x,
        position.y + 2.8f,
        position.z
    };

    DrawCube(
        headPosition,
        2.5f,
        1.5f,
        2.5f,
        bodyColor
    );

    DrawCubeWires(
        headPosition,
        2.5f,
        1.5f,
        2.5f,
        BLACK
    );

    // Sol göz

    DrawSphere(
        Vector3{
            headPosition.x - 0.55f,
            headPosition.y + 0.15f,
            headPosition.z + 1.28f
        },
        0.18f,
        rageModeActive
        ? YELLOW
        : RED
    );

    // Sağ göz

    DrawSphere(
        Vector3{
            headPosition.x + 0.55f,
            headPosition.y + 0.15f,
            headPosition.z + 1.28f
        },
        0.18f,
        rageModeActive
        ? YELLOW
        : RED
    );

    // Boss'un ayakları

    DrawCube(
        Vector3{
            position.x - 0.9f,
            position.y - 2.6f,
            position.z
        },
        1.0f,
        1.4f,
        1.5f,
        bodyColor
    );

    DrawCube(
        Vector3{
            position.x + 0.9f,
            position.y - 2.6f,
            position.z
        },
        1.0f,
        1.4f,
        1.5f,
        bodyColor
    );
}

void Boss::TakeDamage(float damage)
{
    if (
        !alive ||
        damage <= 0.0f
        )
    {
        return;
    }

    health -= damage;

    hitFlashTimer =
        hitFlashDuration;

    if (health <= 0.0f)
    {
        health = 0.0f;
        alive = false;

        currentAttack =
            AttackType::None;

        attackWindupTimer = 0.0f;
    }
}

void Boss::ApplyKnockback(
    Vector3 direction,
    float force
)
{
    if (
        !alive ||
        force <= 0.0f ||
        Vector3Length(direction) <= 0.001f
        )
    {
        return;
    }

    const float bossKnockbackResistance =
        0.25f;

    knockbackVelocity =
        Vector3Scale(
            Vector3Normalize(direction),
            force * bossKnockbackResistance
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