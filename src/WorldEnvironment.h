#pragma once

#include <raylib.h>

class WorldEnvironment
{
public:
    WorldEnvironment();

    void Update(float deltaTime);

    void Draw(
        int currentLevel,
        bool bossFightActive
    ) const;

    bool IsHazardAt(
        int currentLevel,
        Vector3 position
    ) const;

private:
    float animationTime;
};
