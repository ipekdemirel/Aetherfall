#include "ExitDoor.h"

#include <raymath.h>

ExitDoor::ExitDoor(Vector3 startPosition)
    : position(startPosition),
    unlocked(false),
    playerReachedDoor(false),
    activationDistance(2.0f)
{
}

void ExitDoor::Update(
    const Vector3& playerPosition,
    bool isUnlocked
)
{
    unlocked = isUnlocked;

    if (!unlocked)
    {
        playerReachedDoor = false;
        return;
    }

    if (
        Vector3Distance(
            playerPosition,
            position
        ) <= activationDistance
        )
    {
        playerReachedDoor = true;
    }
}

void ExitDoor::Draw() const
{
    Color doorColor =
        unlocked
        ? GREEN
        : RED;

    DrawCube(
        position,
        1.5f,
        3.0f,
        0.4f,
        doorColor
    );

    DrawCubeWires(
        position,
        1.5f,
        3.0f,
        0.4f,
        BLACK
    );
}

bool ExitDoor::IsUnlocked() const
{
    return unlocked;
}

bool ExitDoor::PlayerReachedDoor() const
{
    return playerReachedDoor;
}