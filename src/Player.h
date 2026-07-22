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
    Vector3 facingDirection;

    float walkSpeed;
    float runSpeed;

    float dashSpeed;
    float dashDuration;
    float dashTimer;

    float dashCooldown;
    float dashCooldownTimer;

    bool isDashing;
};