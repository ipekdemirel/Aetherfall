#pragma once

#include <raylib.h>

class Player
{
public:
    Player();

    void Update(float deltaTime);
    void Draw() const;

    Vector3 GetPosition() const;

private:
    Vector3 position;
    float movementSpeed;
};