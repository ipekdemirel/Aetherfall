#pragma once

#include <raylib.h>

class AetherAltar
{
public:
    explicit AetherAltar(Vector3 startPosition);

    bool Update(
        float deltaTime,
        const Vector3& playerPosition,
        bool interactionHeld
    );

    void Draw() const;

    bool IsActivated() const;
    bool IsPlayerInRange() const;

    float GetActivationProgress() const;
    Vector3 GetPosition() const;

private:
    Vector3 position;

    float activationProgress;
    float activationDuration;
    float interactionDistance;
    float effectTime;

    bool activated;
    bool playerInRange;
};