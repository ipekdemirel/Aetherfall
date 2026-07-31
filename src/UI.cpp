#include "UI.h"

#include "Enemy.h"
#include "EnemyManager.h"
#include "Player.h"

#include <cmath>

namespace UI
{
    void DrawHUD(
        const Player& player,
        const char* objectiveName,
        int objectiveProgress,
        int objectiveTotal
    )
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

        // =====================================================
        // PLAYER HP
        // =====================================================

        float healthPercent =
            static_cast<float>(player.GetHealth()) /
            static_cast<float>(player.GetMaxHealth());

        if (healthPercent < 0.0f)
        {
            healthPercent = 0.0f;
        }

        if (healthPercent > 1.0f)
        {
            healthPercent = 1.0f;
        }

        DrawText(
            "PLAYER",
            20,
            190,
            22,
            BLACK
        );

        DrawRectangle(
            20,
            220,
            220,
            24,
            DARKGRAY
        );

        DrawRectangle(
            20,
            220,
            static_cast<int>(
                220.0f * healthPercent
                ),
            24,
            GREEN
        );

        DrawRectangleLines(
            20,
            220,
            220,
            24,
            BLACK
        );

        DrawText(
            TextFormat(
                "%d / %d",
                player.GetHealth(),
                player.GetMaxHealth()
            ),
            80,
            250,
            20,
            BLACK
        );

        // =====================================================
        // CURRENT LEVEL OBJECTIVE
        // =====================================================

        DrawText(
            TextFormat(
                "%s: %d / %d",
                objectiveName,
                objectiveProgress,
                objectiveTotal
            ),
            20,
            290,
            24,
            SKYBLUE
        );

        DrawFPS(
            GetScreenWidth() - 100,
            20
        );
    }

    void DrawAltarInteraction(
        float activationProgress
    )
    {
        if (activationProgress < 0.0f)
        {
            activationProgress = 0.0f;
        }

        if (activationProgress > 1.0f)
        {
            activationProgress = 1.0f;
        }

        const int panelWidth = 440;
        const int panelHeight = 92;
        const int panelX =
            GetScreenWidth() / 2 -
            panelWidth / 2;
        const int panelY =
            GetScreenHeight() -
            panelHeight -
            30;

        DrawRectangleRounded(
            Rectangle{
                static_cast<float>(panelX),
                static_cast<float>(panelY),
                static_cast<float>(panelWidth),
                static_cast<float>(panelHeight)
            },
            0.16f,
            8,
            Color{ 6, 13, 24, 225 }
        );

        DrawRectangleRoundedLinesEx(
            Rectangle{
                static_cast<float>(panelX),
                static_cast<float>(panelY),
                static_cast<float>(panelWidth),
                static_cast<float>(panelHeight)
            },
            0.16f,
            8,
            2.0f,
            Color{ 92, 226, 255, 255 }
        );

        const char* interactionText =
            activationProgress > 0.0f
            ? "HOLD E - CHANNEL AETHER"
            : "HOLD E - ACTIVATE ALTAR";

        const int textWidth =
            MeasureText(
                interactionText,
                22
            );

        DrawText(
            interactionText,
            GetScreenWidth() / 2 -
            textWidth / 2,
            panelY + 15,
            22,
            RAYWHITE
        );

        DrawRectangle(
            panelX + 24,
            panelY + 53,
            panelWidth - 48,
            16,
            Color{ 31, 38, 54, 255 }
        );

        DrawRectangle(
            panelX + 24,
            panelY + 53,
            static_cast<int>(
                (panelWidth - 48) *
                activationProgress
                ),
            16,
            Color{ 92, 226, 255, 255 }
        );

        DrawRectangleLines(
            panelX + 24,
            panelY + 53,
            panelWidth - 48,
            16,
            Color{ 142, 244, 255, 255 }
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

        float healthPercentage =
            enemy.GetHealth() /
            enemy.GetMaxHealth();

        if (healthPercentage < 0.0f)
        {
            healthPercentage = 0.0f;
        }

        if (healthPercentage > 1.0f)
        {
            healthPercentage = 1.0f;
        }

        const float healthBarWidth =
            100.0f;

        const float healthBarHeight =
            12.0f;

        const float healthBarX =
            healthBarScreenPosition.x -
            healthBarWidth / 2.0f;

        const float healthBarY =
            healthBarScreenPosition.y -
            healthBarHeight / 2.0f;

        DrawRectangle(
            static_cast<int>(
                healthBarX - 2.0f
                ),
            static_cast<int>(
                healthBarY - 2.0f
                ),
            static_cast<int>(
                healthBarWidth + 4.0f
                ),
            static_cast<int>(
                healthBarHeight + 4.0f
                ),
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

    void DrawWaveInformation(
        const EnemyManager& enemyManager
    )
    {
        const int panelWidth =
            260;

        const int panelHeight =
            enemyManager.IsWaitingForNextWave()
            ? 145
            : 110;

        const int panelX =
            GetScreenWidth() -
            panelWidth -
            20;

        // Score ve coin panellerinin altında gösterilir.
        const int panelY =
            160;

        DrawRectangle(
            panelX,
            panelY,
            panelWidth,
            panelHeight,
            Color{
                0,
                0,
                0,
                150
            }
        );

        DrawRectangleLines(
            panelX,
            panelY,
            panelWidth,
            panelHeight,
            WHITE
        );

        const char* waveText =
            TextFormat(
                "WAVE %d",
                enemyManager.GetCurrentWave()
            );

        const int waveFontSize =
            30;

        const int waveTextWidth =
            MeasureText(
                waveText,
                waveFontSize
            );

        DrawText(
            waveText,
            panelX +
            panelWidth / 2 -
            waveTextWidth / 2,
            panelY + 14,
            waveFontSize,
            GOLD
        );

        const char* enemiesText =
            TextFormat(
                "Enemies Left: %d",
                enemyManager.GetAliveEnemyCount()
            );

        const int enemiesFontSize =
            22;

        const int enemiesTextWidth =
            MeasureText(
                enemiesText,
                enemiesFontSize
            );

        DrawText(
            enemiesText,
            panelX +
            panelWidth / 2 -
            enemiesTextWidth / 2,
            panelY + 60,
            enemiesFontSize,
            WHITE
        );

        if (
            enemyManager.IsWaitingForNextWave()
            )
        {
            int secondsRemaining =
                static_cast<int>(
                    std::ceil(
                        enemyManager.GetNextWaveTimer()
                    )
                    );

            if (secondsRemaining < 0)
            {
                secondsRemaining = 0;
            }

            const char* nextWaveText =
                TextFormat(
                    "Next Wave: %d",
                    secondsRemaining
                );

            const int timerFontSize =
                24;

            const int timerTextWidth =
                MeasureText(
                    nextWaveText,
                    timerFontSize
                );

            DrawText(
                nextWaveText,
                panelX +
                panelWidth / 2 -
                timerTextWidth / 2,
                panelY + 98,
                timerFontSize,
                SKYBLUE
            );
        }
    }

    void DrawScore(
        int score
    )
    {
        const int panelWidth =
            260;

        const int panelHeight =
            42;

        const int panelX =
            GetScreenWidth() -
            panelWidth -
            20;

        const int panelY =
            60;

        DrawRectangle(
            panelX,
            panelY,
            panelWidth,
            panelHeight,
            Color{
                0,
                0,
                0,
                150
            }
        );

        DrawRectangleLines(
            panelX,
            panelY,
            panelWidth,
            panelHeight,
            WHITE
        );

        const char* scoreText =
            TextFormat(
                "SCORE: %d",
                score
            );

        const int fontSize =
            26;

        const int textWidth =
            MeasureText(
                scoreText,
                fontSize
            );

        DrawText(
            scoreText,
            panelX +
            panelWidth / 2 -
            textWidth / 2,
            panelY + 8,
            fontSize,
            GOLD
        );
    }

    void DrawCoins(
        int coins
    )
    {
        const int panelWidth =
            260;

        const int panelHeight =
            42;

        const int panelX =
            GetScreenWidth() -
            panelWidth -
            20;

        const int panelY =
            110;

        DrawRectangle(
            panelX,
            panelY,
            panelWidth,
            panelHeight,
            Color{
                0,
                0,
                0,
                150
            }
        );

        DrawRectangleLines(
            panelX,
            panelY,
            panelWidth,
            panelHeight,
            GOLD
        );

        const char* coinText =
            TextFormat(
                "COINS: %d",
                coins
            );

        const int fontSize =
            26;

        const int textWidth =
            MeasureText(
                coinText,
                fontSize
            );

        DrawText(
            coinText,
            panelX +
            panelWidth / 2 -
            textWidth / 2,
            panelY + 8,
            fontSize,
            GOLD
        );
    }
}