#include <raylib.h>
#include <raymath.h>

#include "Enemy.h"
#include "Player.h"

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(
        FLAG_MSAA_4X_HINT |
        FLAG_WINDOW_RESIZABLE
    );

    InitWindow(
        screenWidth,
        screenHeight,
        "Aetherfall"
    );

    SetTargetFPS(60);

    Player player;
    Enemy enemy({ 8.0f, 1.0f, -8.0f });

    Camera3D camera{};
    camera.position = { 0.0f, 10.0f, 12.0f };
    camera.target = player.GetPosition();
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const Vector3 cameraOffset{
        0.0f,
        10.0f,
        12.0f
    };

    const float cameraFollowSpeed = 7.0f;
    const float attackRange = 2.5f;
    const float attackDamage = 25.0f;

    bool attackWasActive = false;

    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        player.Update(deltaTime);

        const Vector3 playerPosition = player.GetPosition();

        enemy.Update(deltaTime, playerPosition);

        const bool attackIsActive = player.IsAttacking();

        if (
            attackIsActive &&
            !attackWasActive &&
            enemy.IsAlive()
            )
        {
            const Vector3 enemyPosition = enemy.GetPosition();

            const Vector3 playerToEnemy = Vector3Subtract(
                enemyPosition,
                playerPosition
            );

            const float distanceToEnemy = Vector3Length(
                playerToEnemy
            );

            if (distanceToEnemy <= attackRange)
            {
                const Vector3 directionToEnemy = Vector3Normalize(
                    playerToEnemy
                );

                const float facingAmount = Vector3DotProduct(
                    player.GetFacingDirection(),
                    directionToEnemy
                );

                if (facingAmount > 0.35f)
                {
                    enemy.TakeDamage(attackDamage);
                }
            }
        }

        attackWasActive = attackIsActive;

        const Vector3 desiredCameraTarget{
            playerPosition.x,
            playerPosition.y,
            playerPosition.z
        };

        const Vector3 desiredCameraPosition{
            playerPosition.x + cameraOffset.x,
            playerPosition.y + cameraOffset.y,
            playerPosition.z + cameraOffset.z
        };

        camera.target = Vector3Lerp(
            camera.target,
            desiredCameraTarget,
            cameraFollowSpeed * deltaTime
        );

        camera.position = Vector3Lerp(
            camera.position,
            desiredCameraPosition,
            cameraFollowSpeed * deltaTime
        );

        BeginDrawing();

        ClearBackground(
            Color{ 135, 206, 235, 255 }
        );

        BeginMode3D(camera);

        DrawPlane(
            { 0.0f, 0.0f, 0.0f },
            { 50.0f, 50.0f },
            DARKGREEN
        );

        DrawGrid(
            50,
            1.0f
        );

        player.Draw();
        enemy.Draw();

        EndMode3D();

        if (enemy.IsAlive())
        {
            const Vector3 healthBarWorldPosition{
                enemy.GetPosition().x,
                enemy.GetPosition().y + 1.8f,
                enemy.GetPosition().z
            };

            const Vector2 healthBarScreenPosition = GetWorldToScreen(
                healthBarWorldPosition,
                camera
            );

            const float healthPercentage =
                enemy.GetHealth() / enemy.GetMaxHealth();

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
                    healthBarWidth * healthPercentage
                    ),
                static_cast<int>(healthBarHeight),
                GREEN
            );

            const int healthText = static_cast<int>(
                enemy.GetHealth()
                );

            const char* healthLabel = TextFormat(
                "%d / %d",
                healthText,
                static_cast<int>(enemy.GetMaxHealth())
            );

            const int healthTextWidth = MeasureText(
                healthLabel,
                16
            );

            DrawText(
                healthLabel,
                static_cast<int>(
                    healthBarScreenPosition.x -
                    healthTextWidth / 2
                    ),
                static_cast<int>(
                    healthBarY - 22.0f
                    ),
                16,
                BLACK
            );
        }

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

        EndDrawing();
    }

    CloseWindow();

    return 0;
}