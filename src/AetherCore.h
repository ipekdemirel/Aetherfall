#pragma once

#include <raylib.h>

class AetherCore
{
public:
    AetherCore();

    void Spawn(Vector3 spawnPosition);

    void Update(
        float deltaTime,
        Vector3 playerPosition
    );

    void Draw() const;

    void Reset();

    bool IsSpawned() const;
    bool IsCollected() const;

private:
    Vector3 position;

    float rotationAngle;
    float bobTimer;
    float collectionRange;

    bool spawned;
    bool collected;
};
