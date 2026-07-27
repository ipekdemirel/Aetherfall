#pragma once

#include <raylib.h>

class Coin
{
public:
    Coin(const Vector3& position);

    void Update(float deltaTime, const Vector3& playerPosition);
    void Draw() const;

    bool IsCollected() const;

    Vector3 GetPosition() const;

private:
    Vector3 position;

    float rotation;

    bool collected;

    float collectRadius;
};