#include <raylib.h>
#include <raymath.h>

#include <vector>

#include "CombatSystem.h"
#include "EnemyManager.h"
#include "GameState.h"
#include "Player.h"
#include "UI.h"
#include "KeyItem.h"
#include "ExitDoor.h"

int main()
{
    const int screenWidth =
        1280;

    const int screenHeight =
        720;

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
    std::vector<KeyItem> keys;

    keys.emplace_back(
        Vector3{ -8.0f, 0.5f, -8.0f }
    );

    keys.emplace_back(
        Vector3{ 8.0f, 0.5f, -5.0f }
    );

    keys.emplace_back(
        Vector3{ 0.0f, 0.5f, 8.0f }
    );

    const int totalKeys =
        static_cast<int>(keys.size());

    ExitDoor exitDoor(
        Vector3{ 18.0f, 1.5f, 0.0f }
    );

    EnemyManager enemyManager;
    enemyManager.Initialize();

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
        player.GetPosition().x +
            cameraOffset.x,

        player.GetPosition().y +
            cameraOffset.y,

        player.GetPosition().z +
            cameraOffset.z
    };

    camera.target =
        player.GetPosition();

    camera.up = {
        0.0f,
        1.0f,
        0.0f
    };

    camera.fovy =
        45.0f;

    camera.projection =
        CAMERA_PERSPECTIVE;

    const float cameraFollowSpeed =
        7.0f;

    float cameraShakeTime =
        0.0f;

    const float cameraShakeDuration =
        0.10f;

    const float cameraShakeStrength =
        0.18f;

    int collectedKeys = 0;

    while (!WindowShouldClose())
    {
        const float deltaTime =
            GetFrameTime();

        // =====================================================
        // RESTART
        // =====================================================

        if (
            gameState == GameState::GameOver &&
            IsKeyPressed(KEY_R)
            )
        {
            player =
                Player();

            keys.clear();

            keys.emplace_back(
                Vector3{ -8.0f, 0.5f, -8.0f }
            );

            keys.emplace_back(
                Vector3{ 8.0f, 0.5f, -5.0f }
            );

            keys.emplace_back(
                Vector3{ 0.0f, 0.5f, 8.0f }
            );

            exitDoor =
                ExitDoor(
                    Vector3{ 18.0f, 1.5f, 0.0f }
                );

            enemyManager.Reset();

            enemyManager.Reset();

            combatSystem =
                CombatSystem();

            gameState =
                GameState::Playing;

            cameraShakeTime =
                0.0f;

            camera.position = {
                player.GetPosition().x +
                    cameraOffset.x,

                player.GetPosition().y +
                    cameraOffset.y,

                player.GetPosition().z +
                    cameraOffset.z
            };

            camera.target =
                player.GetPosition();
        }

   // =====================================================
   // UPDATE
   // =====================================================

        if (
            gameState ==
            GameState::Playing
            )
        {
            player.Update(
                deltaTime
            );

            enemyManager.Update(
                deltaTime,
                player.GetPosition()
            );

            combatSystem.Update(
                deltaTime,
                player,
                enemyManager.GetEnemies()
            );

            for (auto& key : keys)
            {
                key.Update(
                    deltaTime,
                    player.GetPosition()
                );
            }

            collectedKeys = 0;

            for (const auto& key : keys)
            {
                if (key.IsCollected())
                {
                    collectedKeys++;
                }
            }

            exitDoor.Update(
                player.GetPosition(),
                collectedKeys == totalKeys
            );

            if (exitDoor.PlayerReachedDoor())
            {
                gameState =
                    GameState::LevelComplete;
            }

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
        // CAMERA FOLLOW
        // =====================================================

        const Vector3 playerPosition =
            player.GetPosition();

        const Vector3 desiredCameraPosition{
            playerPosition.x +
                cameraOffset.x,

            playerPosition.y +
                cameraOffset.y,

            playerPosition.z +
                cameraOffset.z
        };

        camera.target =
            Vector3Lerp(
                camera.target,
                playerPosition,
                cameraFollowSpeed *
                deltaTime
            );

        camera.position =
            Vector3Lerp(
                camera.position,
                desiredCameraPosition,
                cameraFollowSpeed *
                deltaTime
            );

        // =====================================================
        // CAMERA SHAKE
        // =====================================================

        if (cameraShakeTime > 0.0f)
        {
            cameraShakeTime -=
                deltaTime;

            camera.position.x +=
                GetRandomValue(
                    -100,
                    100
                ) /
                100.0f *
                cameraShakeStrength;

            camera.position.y +=
                GetRandomValue(
                    -100,
                    100
                ) /
                100.0f *
                cameraShakeStrength;

            if (cameraShakeTime < 0.0f)
            {
                cameraShakeTime =
                    0.0f;
            }
        }

        // =====================================================
        // DRAW
        // =====================================================

        BeginDrawing();

        ClearBackground(
            SKYBLUE
        );

        BeginMode3D(
            camera
        );

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

        enemyManager.Draw();

        for (const auto& key : keys)
        {
            key.Draw();
        }

        exitDoor.Draw();

        EndMode3D();

        for (
            const Enemy& enemy :
            enemyManager.GetEnemies()
            )
        {
            UI::DrawEnemyHealthBar(
                enemy,
                camera
            );
        }

        UI::DrawHUD(
            player,
            collectedKeys,
            totalKeys
        );

        UI::DrawWaveInformation(
            enemyManager
        );

        // =====================================================
        // GAME OVER
        // =====================================================

        if (
            gameState ==
            GameState::GameOver
            )
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
                GetScreenHeight() / 2 -
                60,
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
                GetScreenHeight() / 2 +
                20,
                restartFontSize,
                WHITE
            );
        }
        // =====================================================
// LEVEL COMPLETE
// =====================================================

        if (
            gameState ==
            GameState::LevelComplete
            )
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
                    170
                }
            );

            const char* completeText =
                "LEVEL COMPLETE";

            const int completeFontSize =
                60;

            const int completeWidth =
                MeasureText(
                    completeText,
                    completeFontSize
                );

            DrawText(
                completeText,
                GetScreenWidth() / 2 -
                completeWidth / 2,
                GetScreenHeight() / 2 -
                70,
                completeFontSize,
                GREEN
            );

            const char* continueText =
                "Press ENTER to Continue";

            const int continueFontSize =
                26;

            const int continueWidth =
                MeasureText(
                    continueText,
                    continueFontSize
                );

            DrawText(
                continueText,
                GetScreenWidth() / 2 -
                continueWidth / 2,
                GetScreenHeight() / 2 +
                20,
                continueFontSize,
                WHITE
            );
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
