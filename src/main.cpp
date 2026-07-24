#include <raylib.h>
#include <raymath.h>

#include <vector>

#include "CombatSystem.h"
#include "Enemy.h"
#include "GameState.h"
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

    // =====================================================
    // DÜŞMANLARIN BAŞLANGIÇ KONUMLARI
    // =====================================================

    const std::vector<Vector3> enemyStartPositions{
        {
            8.0f,
            1.0f,
            -8.0f
        },
        {
            -8.0f,
            1.0f,
            -8.0f
        },
        {
            10.0f,
            1.0f,
            4.0f
        },
        {
            -10.0f,
            1.0f,
            4.0f
        },
        {
            0.0f,
            1.0f,
            -12.0f
        }
    };

    Player player;

    std::vector<Enemy> enemies;

    enemies.reserve(
        enemyStartPositions.size()
    );

    for (
        const Vector3& startPosition :
        enemyStartPositions
        )
    {
        enemies.emplace_back(
            startPosition
        );
    }

    CombatSystem combatSystem;

    GameState gameState =
        GameState::Playing;

    const Vector3 cameraOffset{
        0.0f,
        10.0f,
        12.0f
    };

    Camera3D camera{};

    camera.position = {
        player.GetPosition().x + cameraOffset.x,
        player.GetPosition().y + cameraOffset.y,
        player.GetPosition().z + cameraOffset.z
    };

    camera.target =
        player.GetPosition();

    camera.up = {
        0.0f,
        1.0f,
        0.0f
    };

    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const float cameraFollowSpeed =
        7.0f;

    float cameraShakeTime =
        0.0f;

    const float cameraShakeDuration =
        0.10f;

    const float cameraShakeStrength =
        0.18f;

    while (!WindowShouldClose())
    {
        const float deltaTime =
            GetFrameTime();

        // =====================================================
        // GAME OVER DURUMUNDA YENİDEN BAŞLATMA
        // =====================================================

        if (
            gameState == GameState::GameOver &&
            IsKeyPressed(KEY_R)
            )
        {
            player =
                Player();

            enemies.clear();

            for (
                const Vector3& startPosition :
                enemyStartPositions
                )
            {
                enemies.emplace_back(
                    startPosition
                );
            }

            combatSystem =
                CombatSystem();

            gameState =
                GameState::Playing;

            cameraShakeTime =
                0.0f;

            camera.position = {
                player.GetPosition().x + cameraOffset.x,
                player.GetPosition().y + cameraOffset.y,
                player.GetPosition().z + cameraOffset.z
            };

            camera.target =
                player.GetPosition();
        }

        // =====================================================
        // OYUN GÜNCELLEMELERİ
        // =====================================================

        if (gameState == GameState::Playing)
        {
            player.Update(deltaTime);

            const Vector3 playerPosition =
                player.GetPosition();

            for (Enemy& enemy : enemies)
            {
                enemy.Update(
                    deltaTime,
                    playerPosition
                );
            }

            combatSystem.Update(
                deltaTime,
                player,
                enemies
            );

            if (
                combatSystem
                .ConsumeCameraShakeRequest()
                )
            {
                cameraShakeTime =
                    cameraShakeDuration;
            }

            if (!player.IsAlive())
            {
                gameState =
                    GameState::GameOver;
            }
        }

        // =====================================================
        // KAMERA TAKİBİ
        // =====================================================

        const Vector3 playerPosition =
            player.GetPosition();

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

        // =====================================================
        // KAMERA SARSINTISI
        // =====================================================

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

        // =====================================================
        // ÇİZİM
        // =====================================================

        BeginDrawing();

        ClearBackground(
            Color{
                135,
                206,
                235,
                255
            }
        );

        BeginMode3D(camera);

        DrawPlane(
            {
                0.0f,
                0.0f,
                0.0f
            },
            {
                50.0f,
                50.0f
            },
            DARKGREEN
        );

        DrawGrid(
            50,
            1.0f
        );

        player.Draw();

        for (Enemy& enemy : enemies)
        {
            enemy.Draw();
        }

        EndMode3D();

        for (const Enemy& enemy : enemies)
        {
            UI::DrawEnemyHealthBar(
                enemy,
                camera
            );
        }

        UI::DrawHUD(player);

        // =====================================================
        // GAME OVER EKRANI
        // =====================================================

        if (gameState == GameState::GameOver)
        {
            DrawRectangle(
                0,
                0,
                GetScreenWidth(),
                GetScreenHeight(),
                Color{
                    0,
                    0,
                    0,
                    150
                }
            );

            const char* gameOverText =
                "GAME OVER";

            const int gameOverFontSize =
                60;

            const int gameOverTextWidth =
                MeasureText(
                    gameOverText,
                    gameOverFontSize
                );

            DrawText(
                gameOverText,
                GetScreenWidth() / 2 -
                gameOverTextWidth / 2,
                GetScreenHeight() / 2 - 60,
                gameOverFontSize,
                RED
            );

            const char* restartText =
                "Press R to Restart";

            const int restartFontSize =
                24;

            const int restartTextWidth =
                MeasureText(
                    restartText,
                    restartFontSize
                );

            DrawText(
                restartText,
                GetScreenWidth() / 2 -
                restartTextWidth / 2,
                GetScreenHeight() / 2 + 20,
                restartFontSize,
                WHITE
            );
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}