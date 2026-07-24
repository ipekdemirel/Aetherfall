#pragma once

#include <raylib.h>

class Enemy;
class Player;

namespace UI
{
    void DrawHUD(const Player& player);

    void DrawEnemyHealthBar(
        const Enemy& enemy,
        const Camera3D& camera
    );
}