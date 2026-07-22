#include "Player.h"

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
        }

        return;
    }

    if (Vector3Length(movement) > 0.0f)
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
        0.75f,
        GOLD
    );

    if (weapon.IsAttacking())
    {
        const Vector3 attackPosition{
            position.x + facingDirection.x * 1.5f,
            position.y,
            position.z + facingDirection.z * 1.5f
        };

        DrawCube(
            attackPosition,
            0.35f,
            0.35f,
            2.0f,
            RED
        );

        DrawCubeWires(
            attackPosition,
            0.35f,
            0.35f,
            2.0f,
            MAROON
        );
    }
}

Vector3 Player::GetPosition() const
{
    return position;
}