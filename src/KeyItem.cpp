#include "KeyItem.h"

#include <raymath.h>
#include <cmath>

KeyItem::KeyItem(Vector3 startPosition)
    : position(startPosition),
    collected(false),
    collectionDistance(1.5f),
    rotationAngle(0.0f),
    rotationSpeed(90.0f),
    bobTime(0.0f),
    bobHeight(0.25f)
{
}

void KeyItem::Update(
    float deltaTime,
    const Vector3& playerPosition
)
{
    if (collected)
    {
        return;
    }

    rotationAngle +=
        rotationSpeed *
        deltaTime;

    bobTime +=
        deltaTime;

    const float distance =
        Vector3Distance(
            position,
            playerPosition
        );

    if (distance <= collectionDistance)
    {
        collected = true;
    }
}

void KeyItem::Draw() const
{
    if (collected)
    {
        return;
    }

    Vector3 drawPosition =
        position;

    drawPosition.y +=
        sinf(bobTime * 3.0f) *
        bobHeight;

    DrawCylinder(
        drawPosition,
        0.12f,
        0.12f,
        0.60f,
        16,
        GOLD
    );

    DrawSphere(
        {
            drawPosition.x,
            drawPosition.y + 0.35f,
            drawPosition.z
        },
        0.18f,
        YELLOW
    );
}

bool KeyItem::IsCollected() const
{
    return collected;
}
Vector3 KeyItem::GetPosition() const
{
    return position;
}