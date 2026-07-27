#include "Coin.h"

#include <raymath.h>

Coin::Coin(const Vector3& spawnPosition)
    :
    position(spawnPosition),
    rotation(0.0f),
    collected(false),
    collectRadius(1.2f)
{
}

void Coin::Update(
    float deltaTime,
    const Vector3& playerPosition
)
{
    if (collected)
    {
        return;
    }

    rotation += 180.0f * deltaTime;

    if (rotation >= 360.0f)
    {
        rotation -= 360.0f;
    }

    if (
        Vector3Distance(
            position,
            playerPosition
        ) <= collectRadius
        )
    {
        collected = true;
    }
}

void Coin::Draw() const
{
    if (collected)
    {
        return;
    }

    DrawCylinderEx(
        Vector3{
            position.x,
            position.y - 0.05f,
            position.z
        },
        Vector3{
            position.x,
            position.y + 0.05f,
            position.z
        },
        0.35f,
        0.35f,
        24,
        GOLD
    );

    DrawSphere(
        Vector3{
            position.x,
            position.y + 0.25f,
            position.z
        },
        0.06f,
        YELLOW
    );
}

bool Coin::IsCollected() const
{
    return collected;
}

Vector3 Coin::GetPosition() const
{
    return position;
}