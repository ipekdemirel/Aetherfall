#pragma once

#include <raylib.h>

class Enemy;

namespace UI
{
    void DrawHUD();

    void DrawEnemyHealthBar(
        const Enemy& enemy,
        const Camera3D& camera
    );
}