#include <raylib.h>
#include <raymath.h>

#include "Enemy.h"
#include "Player.h"
#include "UI.h"

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

    float cameraShakeTime = 0.0f;

    const float cameraShakeDuration = 0.10f;
    const float cameraShakeStrength = 0.18f;

    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        player.Update(deltaTime);

        const Vector3 playerPosition =
            player.GetPosition();

        enemy.Update(
            deltaTime,
            playerPosition
        );

        const bool attackIsActive =
            player.IsAttacking();

        if (
            attackIsActive &&
            !attackWasActive &&
            enemy.IsAlive()
            )
        {
            const Vector3 enemyPosition =
                enemy.GetPosition();

            const Vector3 playerToEnemy =
                Vector3Subtract(
                    enemyPosition,
                    playerPosition
                );

            const float distanceToEnemy =
                Vector3Length(
                    playerToEnemy
                );

            if (distanceToEnemy <= attackRange)
            {
                const Vector3 directionToEnemy =
                    Vector3Normalize(
                        playerToEnemy
                    );

                const float facingAmount =
                    Vector3DotProduct(
                        player.GetFacingDirection(),
                        directionToEnemy
                    );

                if (facingAmount > 0.35f)
                {
                    enemy.TakeDamage(
                        attackDamage
                    );

                    enemy.ApplyKnockback(
                        directionToEnemy,
                        5.0f
                    );

                    cameraShakeTime =
                        cameraShakeDuration;
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

        if (cameraShakeTime > 0.0f)
        {
            cameraShakeTime -= deltaTime;

            camera.position.x +=
                GetRandomValue(-100, 100) /
                100.0f *
                cameraShakeStrength;

            camera.position.y +=
                GetRandomValue(-100, 100) /
                100.0f *
                cameraShakeStrength;

            if (cameraShakeTime < 0.0f)
            {
                cameraShakeTime = 0.0f;
            }
        }

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

        UI::DrawEnemyHealthBar(
            enemy,
            camera
        );

        UI::DrawHUD();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}