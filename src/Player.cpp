#include "Player.h"

#include <cmath>

#include <raymath.h>

Player::Player()
    : position{ 0.0f, 1.0f, 0.0f },
    facingDirection{ 0.0f, 0.0f, -1.0f },
    walkSpeed(6.0f),
    runSpeed(9.0f),
    dashSpeed(20.0f),
    dashDuration(0.18f),
    dashTimer(0.0f),
    dashCooldown(0.8f),
    dashCooldownTimer(0.0f),
    isDashing(false)
{
}

void Player::Update(float deltaTime)
{
    weapon.Update(deltaTime);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        weapon.StartAttack();
    }

    if (dashCooldownTimer > 0.0f)
    {
        dashCooldownTimer -= deltaTime;

        if (dashCooldownTimer < 0.0f)
        {
            dashCooldownTimer = 0.0f;
        }
    }

    Vector3 movement{ 0.0f, 0.0f, 0.0f };

    if (IsKeyDown(KEY_W))
    {
        movement.z -= 1.0f;
    }

    if (IsKeyDown(KEY_S))
    {
        movement.z += 1.0f;
    }

    if (IsKeyDown(KEY_A))
    {
        movement.x -= 1.0f;
    }

    if (IsKeyDown(KEY_D))
    {
        movement.x += 1.0f;
    }

    if (!isDashing && Vector3Length(movement) > 0.0f)
    {
        movement = Vector3Normalize(movement);
        facingDirection = movement;
    }

    if (
        IsKeyPressed(KEY_SPACE) &&
        !isDashing &&
        dashCooldownTimer <= 0.0f
        )
    {
        isDashing = true;
        dashTimer = dashDuration;
        dashCooldownTimer = dashCooldown;
    }

    if (isDashing)
    {
        position.x += facingDirection.x * dashSpeed * deltaTime;
        position.z += facingDirection.z * dashSpeed * deltaTime;

        dashTimer -= deltaTime;

        if (dashTimer <= 0.0f)
        {
            isDashing = false;
            dashTimer = 0.0f;
        }
    }
    else if (Vector3Length(movement) > 0.0f)
    {
        const bool isRunning = IsKeyDown(KEY_LEFT_SHIFT);
        const float currentSpeed = isRunning ? runSpeed : walkSpeed;

        position.x += movement.x * currentSpeed * deltaTime;
        position.z += movement.z * currentSpeed * deltaTime;
    }

    const float mapLimit = 23.5f;

    position.x = Clamp(position.x, -mapLimit, mapLimit);
    position.z = Clamp(position.z, -mapLimit, mapLimit);
}

void Player::Draw() const
{
    const Color bodyColor = isDashing ? SKYBLUE : BLUE;

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
        DARKBLUE
    );

    const Vector3 directionMarkerPosition{
        position.x + facingDirection.x * 0.8f,
        position.y,
        position.z + facingDirection.z * 0.8f
    };

    DrawCube(
        directionMarkerPosition,
        0.25f,
        0.25f,
        0.25f,
        GOLD
    );

    // Oyuncunun baktığı yönün sağ tarafını hesaplar.
    const Vector3 rightDirection{
        -facingDirection.z,
        0.0f,
        facingDirection.x
    };

    Vector3 swordDirection = facingDirection;

    float forwardOffset = 0.15f;
    float sideOffset = 0.55f;

    if (weapon.IsAttacking())
    {
        const float attackProgress = weapon.GetAttackProgress();

        // 0 -> 1 -> 0 şeklinde ilerleyen yumuşak saldırı hareketi.
        const float attackPower = sinf(attackProgress * PI);

        forwardOffset += attackPower * 1.0f;
        sideOffset -= attackPower * 0.25f;

        // Kılıca hafif bir sağdan sola savurma hareketi verir.
        const float swingAngle =
            (-0.9f + attackProgress * 1.8f);

        const float cosAngle = cosf(swingAngle);
        const float sinAngle = sinf(swingAngle);

        swordDirection.x =
            facingDirection.x * cosAngle -
            facingDirection.z * sinAngle;

        swordDirection.z =
            facingDirection.x * sinAngle +
            facingDirection.z * cosAngle;

        swordDirection = Vector3Normalize(swordDirection);
    }

    const Vector3 handPosition{
        position.x +
            rightDirection.x * sideOffset +
            facingDirection.x * forwardOffset,

        position.y + 0.45f,

        position.z +
            rightDirection.z * sideOffset +
            facingDirection.z * forwardOffset
    };

    const Vector3 handleEnd{
        handPosition.x + swordDirection.x * 0.45f,
        handPosition.y,
        handPosition.z + swordDirection.z * 0.45f
    };

    const Vector3 bladeStart{
        handleEnd.x + swordDirection.x * 0.10f,
        handleEnd.y,
        handleEnd.z + swordDirection.z * 0.10f
    };

    const Vector3 bladeEnd{
        bladeStart.x + swordDirection.x * 1.45f,
        bladeStart.y,
        bladeStart.z + swordDirection.z * 1.45f
    };

    // Kabza
    DrawCylinderEx(
        handPosition,
        handleEnd,
        0.09f,
        0.09f,
        8,
        BROWN
    );

    // El koruması
    const Vector3 guardStart{
        handleEnd.x - rightDirection.x * 0.30f,
        handleEnd.y,
        handleEnd.z - rightDirection.z * 0.30f
    };

    const Vector3 guardEnd{
        handleEnd.x + rightDirection.x * 0.30f,
        handleEnd.y,
        handleEnd.z + rightDirection.z * 0.30f
    };

    DrawCylinderEx(
        guardStart,
        guardEnd,
        0.07f,
        0.07f,
        8,
        GOLD
    );

    // Kılıç bıçağı
    DrawCylinderEx(
        bladeStart,
        bladeEnd,
        0.10f,
        0.03f,
        8,
        LIGHTGRAY
    );

    // Bıçağın dış çizgisine benzeyen koyu bölüm
    DrawCylinderWiresEx(
        bladeStart,
        bladeEnd,
        0.10f,
        0.03f,
        8,
        DARKGRAY
    );
}

Vector3 Player::GetPosition() const
{
    return position;
}

Vector3 Player::GetFacingDirection() const
{
    return facingDirection;
}

bool Player::IsAttacking() const
{
    return weapon.IsAttacking();
}

float Player::GetAttackProgress() const
{
    return weapon.GetAttackProgress();
}