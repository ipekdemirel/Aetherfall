#pragma once

#include <cmath>

#include <raylib.h>
#include <raymath.h>

#include "Player.h"

class AetherEchoPuzzle
{
public:
    AetherEchoPuzzle()
        : roomIndex(0),
        echoPosition{ 6.0f, 1.0f, 7.0f },
        previousPlayerPosition{ -6.0f, 1.0f, 7.0f },
        synchronizationTimer(0.0f),
        roomMessageTimer(0.0f),
        introductionTimer(0.0f),
        completed(false)
    {
    }

    void Start(Player& player)
    {
        roomIndex = 0;
        completed = false;
        introductionTimer = 7.0f;
        roomMessageTimer = 0.0f;
        ResetCurrentRoom(player);
    }

    void ResetCurrentRoom(Player& player)
    {
        const Vector3 playerStart = GetPlayerStart();

        player.SetPosition(playerStart);
        previousPlayerPosition = playerStart;
        echoPosition = GetEchoStart();
        synchronizationTimer = 0.0f;
    }

    void Update(float deltaTime, Player& player)
    {
        if (completed)
        {
            return;
        }

        if (introductionTimer > 0.0f)
        {
            introductionTimer -= deltaTime;
        }

        if (roomMessageTimer > 0.0f)
        {
            roomMessageTimer -= deltaTime;
        }

        if (IsKeyPressed(KEY_R))
        {
            ResetCurrentRoom(player);
            return;
        }

        Vector3 playerPosition = player.GetPosition();
        playerPosition.y = 1.0f;

        playerPosition.x = Clamp(playerPosition.x, -10.5f, -0.9f);
        playerPosition.z = Clamp(playerPosition.z, -9.0f, 9.0f);

        if (IsBlocked(playerPosition, false))
        {
            playerPosition = previousPlayerPosition;
        }

        player.SetPosition(playerPosition);

        const Vector3 playerMovement =
            Vector3Subtract(playerPosition, previousPlayerPosition);

        Vector3 echoCandidate{
            echoPosition.x - playerMovement.x,
            1.0f,
            echoPosition.z + playerMovement.z
        };

        echoCandidate.x = Clamp(echoCandidate.x, 0.9f, 10.5f);
        echoCandidate.z = Clamp(echoCandidate.z, -9.0f, 9.0f);

        if (!IsBlocked(echoCandidate, true))
        {
            echoPosition = echoCandidate;
        }

        previousPlayerPosition = playerPosition;

        const bool playerOnPad =
            HorizontalDistance(playerPosition, GetPlayerPad()) <= 1.25f;

        const bool echoOnPad =
            HorizontalDistance(echoPosition, GetEchoPad()) <= 1.25f;

        if (playerOnPad && echoOnPad)
        {
            synchronizationTimer += deltaTime;

            if (synchronizationTimer >= 0.65f)
            {
                ++roomIndex;
                roomMessageTimer = 2.0f;

                if (roomIndex >= 3)
                {
                    completed = true;
                }
                else
                {
                    ResetCurrentRoom(player);
                }
            }
        }
        else
        {
            synchronizationTimer = 0.0f;
        }
    }

    void DrawWorld() const
    {
        DrawPlane(
            Vector3{ 0.0f, 0.0f, 0.0f },
            Vector2{ 23.0f, 20.0f },
            Color{ 9, 24, 37, 255 }
        );

        for (int line = -10; line <= 10; line += 2)
        {
            DrawLine3D(
                Vector3{ static_cast<float>(line), 0.025f, -9.5f },
                Vector3{ static_cast<float>(line), 0.025f, 9.5f },
                Color{ 28, 71, 91, 150 }
            );

            DrawLine3D(
                Vector3{ -11.0f, 0.025f, static_cast<float>(line) },
                Vector3{ 11.0f, 0.025f, static_cast<float>(line) },
                Color{ 28, 71, 91, 150 }
            );
        }

        DrawCube(
            Vector3{ 0.0f, 1.3f, 0.0f },
            1.0f,
            2.6f,
            20.0f,
            Color{ 18, 48, 67, 255 }
        );

        DrawCubeWires(
            Vector3{ 0.0f, 1.3f, 0.0f },
            1.0f,
            2.6f,
            20.0f,
            Color{ 59, 193, 226, 210 }
        );

        DrawPad(GetPlayerPad(), Color{ 255, 188, 67, 255 });
        DrawPad(GetEchoPad(), Color{ 76, 225, 255, 255 });

        DrawObstacles(false);
        DrawObstacles(true);

        DrawEcho();

        DrawSphere(
            Vector3{ 0.0f, 3.1f, -7.6f },
            0.38f,
            Color{ 86, 228, 255, 220 }
        );

        DrawSphereWires(
            Vector3{ 0.0f, 3.1f, -7.6f },
            0.62f,
            10,
            10,
            Color{ 162, 246, 255, 180 }
        );
    }

    void DrawHUD() const
    {
        const int displayedRoom = completed ? 3 : roomIndex + 1;

        const char* title = TextFormat(
            "AETHER ECHO - CHAMBER %d/3",
            displayedRoom
        );

        DrawText(
            title,
            GetScreenWidth() / 2 - MeasureText(title, 26) / 2,
            82,
            26,
            Color{ 96, 229, 255, 255 }
        );

        const char* objective =
            "PLACE BOTH HERO AND ECHO ON THEIR MATCHING LIGHT PADS";

        DrawText(
            objective,
            GetScreenWidth() / 2 - MeasureText(objective, 18) / 2,
            114,
            18,
            RAYWHITE
        );

        const char* controls =
            "WASD: MOVE   SPACE: DASH   R: RESET CHAMBER";

        DrawText(
            controls,
            GetScreenWidth() / 2 - MeasureText(controls, 17) / 2,
            GetScreenHeight() - 38,
            17,
            Color{ 190, 218, 229, 255 }
        );

        const bool playerReady =
            HorizontalDistance(previousPlayerPosition, GetPlayerPad()) <= 1.25f;

        const bool echoReady =
            HorizontalDistance(echoPosition, GetEchoPad()) <= 1.25f;

        const char* status = TextFormat(
            "HERO: %s     ECHO: %s",
            playerReady ? "READY" : "SEARCHING",
            echoReady ? "READY" : "SEARCHING"
        );

        DrawText(
            status,
            GetScreenWidth() / 2 - MeasureText(status, 21) / 2,
            142,
            21,
            playerReady && echoReady ? GREEN : GOLD
        );

        if (introductionTimer > 0.0f)
        {
            DrawPanel(
                "YOUR ECHO MIRRORS LEFT AND RIGHT",
                "GUIDE BOTH FIGURES TO THE TWO GLOWING PADS"
            );
        }
        else if (roomMessageTimer > 0.0f)
        {
            DrawPanel(
                "CHAMBER SYNCHRONIZED",
                "THE AETHER PATH HAS SHIFTED"
            );
        }
    }

    bool IsComplete() const
    {
        return completed;
    }

    int GetCompletedRoomCount() const
    {
        return completed ? 3 : roomIndex;
    }

    Vector3 GetEchoPosition() const
    {
        return echoPosition;
    }

private:
    struct Barrier
    {
        Vector3 center;
        Vector3 size;
    };

    static float HorizontalDistance(Vector3 first, Vector3 second)
    {
        const float x = first.x - second.x;
        const float z = first.z - second.z;
        return std::sqrt(x * x + z * z);
    }

    Vector3 GetPlayerStart() const
    {
        if (roomIndex == 1)
        {
            return Vector3{ -8.0f, 1.0f, 7.0f };
        }

        if (roomIndex == 2)
        {
            return Vector3{ -7.5f, 1.0f, 7.5f };
        }

        return Vector3{ -6.0f, 1.0f, 7.0f };
    }

    Vector3 GetEchoStart() const
    {
        if (roomIndex == 1)
        {
            return Vector3{ 8.0f, 1.0f, 7.0f };
        }

        if (roomIndex == 2)
        {
            return Vector3{ 7.5f, 1.0f, 7.5f };
        }

        return Vector3{ 6.0f, 1.0f, 7.0f };
    }

    Vector3 GetPlayerPad() const
    {
        if (roomIndex == 1)
        {
            return Vector3{ -3.0f, 0.08f, -6.5f };
        }

        if (roomIndex == 2)
        {
            return Vector3{ -8.5f, 0.08f, -7.0f };
        }

        return Vector3{ -3.0f, 0.08f, -5.5f };
    }

    Vector3 GetEchoPad() const
    {
        if (roomIndex == 1)
        {
            return Vector3{ 3.0f, 0.08f, -6.5f };
        }

        if (roomIndex == 2)
        {
            return Vector3{ 8.5f, 0.08f, -7.0f };
        }

        return Vector3{ 3.0f, 0.08f, -5.5f };
    }

    int GetBarriers(bool echoSide, Barrier* barriers) const
    {
        int count = 0;

        if (roomIndex == 1)
        {
            if (!echoSide)
            {
                barriers[count++] = {
                    Vector3{ -6.0f, 1.0f, -1.0f },
                    Vector3{ 5.7f, 2.0f, 1.0f }
                };
            }
            else
            {
                barriers[count++] = {
                    Vector3{ 4.0f, 1.0f, 1.5f },
                    Vector3{ 1.0f, 2.0f, 6.5f }
                };
            }
        }
        else if (roomIndex == 2)
        {
            if (!echoSide)
            {
                barriers[count++] = {
                    Vector3{ -5.0f, 1.0f, 2.0f },
                    Vector3{ 1.0f, 2.0f, 8.5f }
                };

                barriers[count++] = {
                    Vector3{ -7.5f, 1.0f, -3.0f },
                    Vector3{ 5.0f, 2.0f, 1.0f }
                };
            }
            else
            {
                barriers[count++] = {
                    Vector3{ 7.0f, 1.0f, 2.0f },
                    Vector3{ 1.0f, 2.0f, 8.5f }
                };

                barriers[count++] = {
                    Vector3{ 4.8f, 1.0f, -3.0f },
                    Vector3{ 4.4f, 2.0f, 1.0f }
                };
            }
        }

        return count;
    }

    bool IsBlocked(Vector3 position, bool echoSide) const
    {
        Barrier barriers[3]{};
        const int count = GetBarriers(echoSide, barriers);

        for (int index = 0; index < count; ++index)
        {
            const Barrier& barrier = barriers[index];
            const float halfWidth = barrier.size.x * 0.5f + 0.42f;
            const float halfDepth = barrier.size.z * 0.5f + 0.42f;

            if (
                std::fabs(position.x - barrier.center.x) <= halfWidth &&
                std::fabs(position.z - barrier.center.z) <= halfDepth
                )
            {
                return true;
            }
        }

        return false;
    }

    void DrawObstacles(bool echoSide) const
    {
        Barrier barriers[3]{};
        const int count = GetBarriers(echoSide, barriers);

        for (int index = 0; index < count; ++index)
        {
            const Barrier& barrier = barriers[index];

            DrawCube(
                barrier.center,
                barrier.size.x,
                barrier.size.y,
                barrier.size.z,
                echoSide
                ? Color{ 20, 105, 128, 205 }
                : Color{ 128, 72, 30, 220 }
            );

            DrawCubeWires(
                barrier.center,
                barrier.size.x,
                barrier.size.y,
                barrier.size.z,
                echoSide
                ? Color{ 101, 232, 255, 255 }
                : Color{ 255, 188, 79, 255 }
            );
        }
    }

    static void DrawPad(Vector3 position, Color color)
    {
        DrawCylinder(position, 1.45f, 1.45f, 0.14f, 32, Fade(color, 0.45f));
        DrawCylinderWires(position, 1.45f, 1.45f, 0.14f, 32, color);
        DrawCircle3D(
            Vector3{ position.x, position.y + 0.09f, position.z },
            0.75f,
            Vector3{ 1.0f, 0.0f, 0.0f },
            90.0f,
            Fade(color, 0.85f)
        );
    }

    void DrawEcho() const
    {
        const Color echoColor{ 73, 220, 255, 145 };
        const Color glowColor{ 154, 245, 255, 90 };

        DrawCylinderEx(
            Vector3{ echoPosition.x, 0.45f, echoPosition.z },
            Vector3{ echoPosition.x, 1.75f, echoPosition.z },
            0.43f,
            0.30f,
            12,
            echoColor
        );

        DrawSphere(
            Vector3{ echoPosition.x, 2.05f, echoPosition.z },
            0.34f,
            echoColor
        );

        DrawSphereWires(
            Vector3{ echoPosition.x, 1.25f, echoPosition.z },
            0.75f,
            10,
            10,
            glowColor
        );
    }

    static void DrawPanel(const char* firstLine, const char* secondLine)
    {
        const int width = 570;
        const int height = 118;
        const int x = GetScreenWidth() / 2 - width / 2;
        const int y = GetScreenHeight() / 2 - height / 2;

        DrawRectangle(x, y, width, height, Color{ 3, 15, 25, 228 });
        DrawRectangleLinesEx(
            Rectangle{
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(width),
                static_cast<float>(height)
            },
            3.0f,
            Color{ 82, 224, 255, 255 }
        );

        DrawText(
            firstLine,
            GetScreenWidth() / 2 - MeasureText(firstLine, 28) / 2,
            y + 20,
            28,
            SKYBLUE
        );

        DrawText(
            secondLine,
            GetScreenWidth() / 2 - MeasureText(secondLine, 18) / 2,
            y + 70,
            18,
            RAYWHITE
        );
    }

    int roomIndex;
    Vector3 echoPosition;
    Vector3 previousPlayerPosition;
    float synchronizationTimer;
    float roomMessageTimer;
    float introductionTimer;
    bool completed;
};
