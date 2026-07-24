#include "UI.h"

#include "Enemy.h"

namespace UI
{
    void DrawHUD()
    {
        DrawText(
            "AETHERFALL",
            20,
            20,
            28,
            BLACK
        );

        DrawText(
            "WASD: Move",
            20,
            58,
            20,
            DARKGRAY
        );

        DrawText(
            "LEFT SHIFT: Run",
            20,
            86,
            20,
            DARKGRAY
        );

        DrawText(
            "SPACE: Dash",
            20,
            114,
            20,
            DARKGRAY
        );

        DrawText(
            "LEFT MOUSE: Attack",
            20,
            142,
            20,
            DARKGRAY
        );

        DrawFPS(
            GetScreenWidth() - 100,
            20
        );
    }

    void DrawEnemyHealthBar(
        const Enemy& enemy,
        const Camera3D& camera
    )
    {
        if (!enemy.IsAlive())
        {
            return;
        }

        const Vector3 enemyPosition =
            enemy.GetPosition();

        const Vector3 healthBarWorldPosition{
            enemyPosition.x,
            enemyPosition.y + 1.8f,
            enemyPosition.z
        };

        const Vector2 healthBarScreenPosition =
            GetWorldToScreen(
                healthBarWorldPosition,
                camera
            );

        const float healthPercentage =
            enemy.GetHealth() /
            enemy.GetMaxHealth();

        const float healthBarWidth = 100.0f;
        const float healthBarHeight = 12.0f;

        const float healthBarX =
            healthBarScreenPosition.x -
            healthBarWidth / 2.0f;

        const float healthBarY =
            healthBarScreenPosition.y -
            healthBarHeight / 2.0f;

        DrawRectangle(
            static_cast<int>(healthBarX - 2.0f),
            static_cast<int>(healthBarY - 2.0f),
            static_cast<int>(healthBarWidth + 4.0f),
            static_cast<int>(healthBarHeight + 4.0f),
            BLACK
        );

        DrawRectangle(
            static_cast<int>(healthBarX),
            static_cast<int>(healthBarY),
            static_cast<int>(healthBarWidth),
            static_cast<int>(healthBarHeight),
            DARKGRAY
        );

        DrawRectangle(
            static_cast<int>(healthBarX),
            static_cast<int>(healthBarY),
            static_cast<int>(
                healthBarWidth *
                healthPercentage
                ),
            static_cast<int>(healthBarHeight),
            GREEN
        );

        const char* healthLabel =
            TextFormat(
                "%d / %d",
                static_cast<int>(
                    enemy.GetHealth()
                    ),
                static_cast<int>(
                    enemy.GetMaxHealth()
                    )
            );

        const int textWidth =
            MeasureText(
                healthLabel,
                16
            );

        DrawText(
            healthLabel,
            static_cast<int>(
                healthBarScreenPosition.x -
                textWidth / 2
                ),
            static_cast<int>(
                healthBarY - 22.0f
                ),
            16,
            BLACK
        );
    }
}