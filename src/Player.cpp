#include "Player.h"

#include <raymath.h>

Player::Player()
    : position{ 0.0f, 1.0f, 0.0f },
    movementSpeed(6.0f)
{
}

void Player::Update(float deltaTime)
{
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

    if (Vector3Length(movement) > 0.0f)
    {
        movement = Vector3Normalize(movement);

        position.x += movement.x * movementSpeed * deltaTime;
        position.z += movement.z * movementSpeed * deltaTime;
    }
}

void Player::Draw() const
{
    DrawCube(
        position,
        1.0f,
        2.0f,
        1.0f,
        BLUE
    );

    DrawCubeWires(
        position,
        1.0f,
        2.0f,
        1.0f,
        DARKBLUE
    );
}

Vector3 Player::GetPosition() const
{
    return position;
}