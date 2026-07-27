#pragma once

#include <raylib.h>

class ExitDoor
{
public:
    ExitDoor(Vector3 position);

    void Update(
        const Vector3& playerPosition,
        bool unlocked
    );

    void Draw() const;

    bool IsUnlocked() const;

    bool PlayerReachedDoor() const;

private:
    Vector3 position;

    bool unlocked;

    bool playerReachedDoor;

    float activationDistance;
};