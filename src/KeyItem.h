#pragma once

#include <raylib.h>

class KeyItem
{
public:
    KeyItem(Vector3 startPosition);

    void Update(
        float deltaTime,
        const Vector3& playerPosition
    );

    void Draw() const;

    bool IsCollected() const;

    Vector3 GetPosition() const;

private:
    Vector3 position;

    bool collected;

    float collectionDistance;

    float rotationAngle;
    float rotationSpeed;

    float bobTime;
    float bobHeight;
};