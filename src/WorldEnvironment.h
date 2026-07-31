#pragma once

class WorldEnvironment
{
public:
    WorldEnvironment();

    void Update(float deltaTime);

    void Draw(
        int currentLevel,
        bool bossFightActive
    ) const;

private:
    float animationTime;
};
