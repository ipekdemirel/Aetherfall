#pragma once

#include <array>
#include <cmath>

#include <raylib.h>
#include <raymath.h>

#include "Player.h"

class AetherColorMatchPuzzle
{
public:
    AetherColorMatchPuzzle()
        : targetColorIndex(0),
        correctHitsInMatch(0),
        completedMatches(0),
        boardGeneration(0),
        wasAttacking(false),
        doorOpen(false),
        completed(false),
        introductionTimer(7.0f),
        feedbackTimer(0.0f),
        boardTransitionTimer(0.0f),
        feedbackType(FeedbackType::None)
    {
        BuildBoard();
    }

    void Start(Player& player)
    {
        targetColorIndex = 0;
        correctHitsInMatch = 0;
        completedMatches = 0;
        boardGeneration = 0;
        wasAttacking = false;
        doorOpen = false;
        completed = false;
        introductionTimer = 7.0f;
        feedbackTimer = 0.0f;
        boardTransitionTimer = 0.0f;
        feedbackType = FeedbackType::None;

        player.SetPosition(Vector3{ 0.0f, 1.0f, 9.0f });
        BuildBoard();
    }

    void Update(float deltaTime, Player& player)
    {
        if (completed)
        {
            return;
        }

        introductionTimer = MaxZero(introductionTimer - deltaTime);
        feedbackTimer = MaxZero(feedbackTimer - deltaTime);

        Vector3 playerPosition = player.GetPosition();
        playerPosition.y = 1.0f;
        playerPosition.x = Clamp(playerPosition.x, -10.4f, 10.4f);
        playerPosition.z = Clamp(
            playerPosition.z,
            doorOpen ? -12.0f : -8.6f,
            10.5f
        );
        player.SetPosition(playerPosition);

        if (boardTransitionTimer > 0.0f)
        {
            boardTransitionTimer -= deltaTime;

            if (boardTransitionTimer <= 0.0f && !doorOpen)
            {
                ++boardGeneration;
                targetColorIndex =
                    (targetColorIndex + 1 + boardGeneration % 2) % ColorCount;
                correctHitsInMatch = 0;
                BuildBoard();
            }
        }

        const bool attacking = player.IsAttacking();

        if (
            attacking &&
            !wasAttacking &&
            boardTransitionTimer <= 0.0f &&
            !doorOpen
            )
        {
            TryHitOrb(player);
        }

        wasAttacking = attacking;

        if (
            doorOpen &&
            HorizontalDistance(playerPosition, DoorPosition()) <= 1.75f
            )
        {
            completed = true;
        }
    }

    void DrawWorld() const
    {
        DrawPlane(
            Vector3{ 0.0f, 0.0f, 0.0f },
            Vector2{ 23.0f, 23.0f },
            Color{ 11, 21, 34, 255 }
        );

        for (int line = -10; line <= 10; line += 2)
        {
            DrawLine3D(
                Vector3{ static_cast<float>(line), 0.025f, -10.5f },
                Vector3{ static_cast<float>(line), 0.025f, 10.5f },
                Color{ 34, 63, 86, 150 }
            );
            DrawLine3D(
                Vector3{ -10.5f, 0.025f, static_cast<float>(line) },
                Vector3{ 10.5f, 0.025f, static_cast<float>(line) },
                Color{ 34, 63, 86, 150 }
            );
        }

        DrawArenaBorder();
        DrawTargetCrystal();

        const float time = static_cast<float>(GetTime());

        for (int index = 0; index < OrbCount; ++index)
        {
            const Orb& orb = orbs[index];

            if (!orb.active)
            {
                continue;
            }

            const float bob =
                std::sin(time * 2.4f + static_cast<float>(index)) * 0.12f;
            const Vector3 position{
                orb.position.x,
                orb.position.y + bob,
                orb.position.z
            };
            const Color color = OrbColor(orb.colorIndex);

            DrawCylinder(
                Vector3{ position.x, 0.13f, position.z },
                0.82f,
                0.62f,
                0.20f,
                24,
                Fade(color, 0.28f)
            );
            DrawSphere(position, 0.62f, color);
            DrawSphereWires(position, 0.73f, 12, 12, Fade(RAYWHITE, 0.72f));
            DrawSphere(position, 0.23f, Fade(RAYWHITE, 0.55f));
        }

        DrawDoor();
    }

    void DrawHUD() const
    {
        const char* title = "AETHER COLOR VAULT";
        DrawText(
            title,
            GetScreenWidth() / 2 - MeasureText(title, 28) / 2,
            82,
            28,
            Color{ 112, 229, 255, 255 }
        );

        const char* targetText = TextFormat(
            "TARGET: %s    MATCH: %d/3    PATTERNS: %d/%d",
            ColorName(targetColorIndex),
            correctHitsInMatch,
            completedMatches,
            RequiredMatches
        );
        DrawText(
            targetText,
            GetScreenWidth() / 2 - MeasureText(targetText, 22) / 2,
            118,
            22,
            OrbColor(targetColorIndex)
        );

        const char* instruction = doorOpen
            ? "THE NORTH GATE IS OPEN - WALK THROUGH IT"
            : "STRIKE 3 ORBS OF THE TARGET COLOR WITH LEFT MOUSE";
        DrawText(
            instruction,
            GetScreenWidth() / 2 - MeasureText(instruction, 18) / 2,
            150,
            18,
            doorOpen ? GREEN : RAYWHITE
        );

        if (introductionTimer > 0.0f)
        {
            DrawPanel(
                "MATCH THE AETHER COLORS",
                "BREAK THREE TARGET ORBS - COMPLETE FIVE PATTERNS"
            );
        }
        else if (feedbackTimer > 0.0f)
        {
            if (feedbackType == FeedbackType::Correct)
            {
                DrawPanel("CORRECT COLOR", "KEEP THE THREE-HIT MATCH GOING");
            }
            else if (feedbackType == FeedbackType::Wrong)
            {
                DrawPanel("WRONG COLOR", "LOOK AT THE TARGET COLOR ABOVE");
            }
            else if (feedbackType == FeedbackType::PatternComplete)
            {
                DrawPanel("PATTERN COMPLETE", "THE VAULT IS RESHUFFLING");
            }
            else if (feedbackType == FeedbackType::DoorOpened)
            {
                DrawPanel("ALL PATTERNS COMPLETE", "THE NORTH GATE IS NOW OPEN");
            }
        }

        const char* controls = "WASD: MOVE   LEFT SHIFT: RUN   SPACE: DASH   LEFT MOUSE: STRIKE";
        DrawText(
            controls,
            GetScreenWidth() / 2 - MeasureText(controls, 17) / 2,
            GetScreenHeight() - 38,
            17,
            Color{ 190, 218, 229, 255 }
        );
    }

    bool IsComplete() const
    {
        return completed;
    }

    int GetCompletedMatchCount() const
    {
        return completedMatches;
    }

    static int GetRequiredMatchCount()
    {
        return RequiredMatches;
    }

private:
    static constexpr int OrbCount = 16;
    static constexpr int ColorCount = 4;
    static constexpr int RequiredMatches = 5;

    enum class FeedbackType
    {
        None,
        Correct,
        Wrong,
        PatternComplete,
        DoorOpened
    };

    struct Orb
    {
        Vector3 position;
        int colorIndex;
        bool active;
    };

    std::array<Orb, OrbCount> orbs{};
    int targetColorIndex;
    int correctHitsInMatch;
    int completedMatches;
    int boardGeneration;
    bool wasAttacking;
    bool doorOpen;
    bool completed;
    float introductionTimer;
    float feedbackTimer;
    float boardTransitionTimer;
    FeedbackType feedbackType;

    static float MaxZero(float value)
    {
        return value < 0.0f ? 0.0f : value;
    }

    static float HorizontalDistance(Vector3 first, Vector3 second)
    {
        const float x = first.x - second.x;
        const float z = first.z - second.z;
        return std::sqrt(x * x + z * z);
    }

    static Vector3 DoorPosition()
    {
        return Vector3{ 0.0f, 1.5f, -10.6f };
    }

    static Color OrbColor(int colorIndex)
    {
        static const Color colors[ColorCount]{
            Color{ 255, 74, 96, 255 },
            Color{ 63, 190, 255, 255 },
            Color{ 255, 203, 61, 255 },
            Color{ 102, 235, 137, 255 }
        };
        return colors[colorIndex % ColorCount];
    }

    static const char* ColorName(int colorIndex)
    {
        static const char* names[ColorCount]{
            "RED",
            "BLUE",
            "YELLOW",
            "GREEN"
        };
        return names[colorIndex % ColorCount];
    }

    void BuildBoard()
    {
        int index = 0;

        for (int row = 0; row < 4; ++row)
        {
            for (int column = 0; column < 4; ++column)
            {
                const int color =
                    (row * 3 + column * 2 + boardGeneration) % ColorCount;

                orbs[index] = Orb{
                    Vector3{
                        -7.5f + static_cast<float>(column) * 5.0f,
                        0.92f,
                        -6.0f + static_cast<float>(row) * 4.4f
                    },
                    color,
                    true
                };
                ++index;
            }
        }

        // Her yeni tahtada hedef renkten kesinlikle en az alti top bulunur.
        const int forcedTargetIndices[6]{ 0, 3, 5, 10, 12, 15 };
        for (int forcedIndex : forcedTargetIndices)
        {
            orbs[forcedIndex].colorIndex = targetColorIndex;
        }
    }

    void TryHitOrb(const Player& player)
    {
        Vector3 forward = player.GetFacingDirection();
        forward.y = 0.0f;

        if (Vector3Length(forward) < 0.01f)
        {
            forward = Vector3{ 0.0f, 0.0f, -1.0f };
        }
        else
        {
            forward = Vector3Normalize(forward);
        }

        const Vector3 playerPosition = player.GetPosition();
        int nearestIndex = -1;
        float nearestDistance = 2.35f;

        for (int index = 0; index < OrbCount; ++index)
        {
            const Orb& orb = orbs[index];

            if (!orb.active)
            {
                continue;
            }

            Vector3 toOrb = Vector3Subtract(orb.position, playerPosition);
            toOrb.y = 0.0f;
            const float distance = Vector3Length(toOrb);

            if (distance <= 0.01f || distance >= nearestDistance)
            {
                continue;
            }

            const float facingDot =
                Vector3DotProduct(forward, Vector3Scale(toOrb, 1.0f / distance));

            if (facingDot >= -0.05f)
            {
                nearestIndex = index;
                nearestDistance = distance;
            }
        }

        if (nearestIndex < 0)
        {
            return;
        }

        Orb& hitOrb = orbs[nearestIndex];

        if (hitOrb.colorIndex != targetColorIndex)
        {
            feedbackType = FeedbackType::Wrong;
            feedbackTimer = 1.15f;
            return;
        }

        hitOrb.active = false;
        ++correctHitsInMatch;
        feedbackType = FeedbackType::Correct;
        feedbackTimer = 0.85f;

        if (correctHitsInMatch < 3)
        {
            return;
        }

        ++completedMatches;

        if (completedMatches >= RequiredMatches)
        {
            doorOpen = true;
            correctHitsInMatch = 3;
            feedbackType = FeedbackType::DoorOpened;
            feedbackTimer = 4.0f;
        }
        else
        {
            feedbackType = FeedbackType::PatternComplete;
            feedbackTimer = 1.6f;
            boardTransitionTimer = 1.25f;
        }
    }

    void DrawArenaBorder() const
    {
        const Color wallColor{ 20, 46, 67, 255 };
        const Color wireColor{ 59, 193, 226, 220 };

        DrawCube(Vector3{ -11.0f, 1.0f, 0.0f }, 0.5f, 2.0f, 22.0f, wallColor);
        DrawCubeWires(Vector3{ -11.0f, 1.0f, 0.0f }, 0.5f, 2.0f, 22.0f, wireColor);
        DrawCube(Vector3{ 11.0f, 1.0f, 0.0f }, 0.5f, 2.0f, 22.0f, wallColor);
        DrawCubeWires(Vector3{ 11.0f, 1.0f, 0.0f }, 0.5f, 2.0f, 22.0f, wireColor);
        DrawCube(Vector3{ 0.0f, 1.0f, 11.0f }, 22.0f, 2.0f, 0.5f, wallColor);
        DrawCubeWires(Vector3{ 0.0f, 1.0f, 11.0f }, 22.0f, 2.0f, 0.5f, wireColor);

        DrawCube(Vector3{ -6.8f, 1.0f, -11.0f }, 8.0f, 2.0f, 0.5f, wallColor);
        DrawCube(Vector3{ 6.8f, 1.0f, -11.0f }, 8.0f, 2.0f, 0.5f, wallColor);
    }

    void DrawTargetCrystal() const
    {
        const Color target = OrbColor(targetColorIndex);
        DrawCylinder(Vector3{ 0.0f, 0.12f, 1.0f }, 1.25f, 1.0f, 0.22f, 32, Fade(target, 0.30f));
        DrawSphere(Vector3{ 0.0f, 0.55f, 1.0f }, 0.28f, target);
        DrawSphereWires(Vector3{ 0.0f, 0.55f, 1.0f }, 0.48f, 10, 10, Fade(RAYWHITE, 0.80f));
    }

    void DrawDoor() const
    {
        const Vector3 door = DoorPosition();
        const Color frame = doorOpen
            ? Color{ 76, 235, 141, 255 }
            : Color{ 48, 69, 86, 255 };

        DrawCube(Vector3{ door.x - 2.0f, 1.8f, door.z }, 0.55f, 3.6f, 0.65f, frame);
        DrawCube(Vector3{ door.x + 2.0f, 1.8f, door.z }, 0.55f, 3.6f, 0.65f, frame);
        DrawCube(Vector3{ door.x, 3.55f, door.z }, 4.55f, 0.45f, 0.65f, frame);

        if (doorOpen)
        {
            DrawCube(
                Vector3{ door.x, 1.75f, door.z + 0.02f },
                3.3f,
                3.0f,
                0.08f,
                Fade(Color{ 82, 255, 160, 255 }, 0.45f)
            );
            DrawSphere(Vector3{ door.x, 1.7f, door.z }, 0.25f, RAYWHITE);
        }
        else
        {
            DrawCube(Vector3{ door.x, 1.7f, door.z }, 3.35f, 3.0f, 0.35f, Color{ 23, 34, 48, 255 });
            DrawCubeWires(Vector3{ door.x, 1.7f, door.z }, 3.35f, 3.0f, 0.35f, Color{ 101, 125, 146, 255 });
        }
    }

    static void DrawPanel(const char* title, const char* subtitle)
    {
        const int panelWidth = 620;
        const int panelHeight = 116;
        const int panelX = GetScreenWidth() / 2 - panelWidth / 2;
        const int panelY = GetScreenHeight() / 2 - panelHeight / 2;

        DrawRectangle(panelX, panelY, panelWidth, panelHeight, Color{ 5, 14, 23, 225 });
        DrawRectangleLinesEx(
            Rectangle{
                static_cast<float>(panelX),
                static_cast<float>(panelY),
                static_cast<float>(panelWidth),
                static_cast<float>(panelHeight)
            },
            2.0f,
            Color{ 84, 218, 245, 255 }
        );
        DrawText(title, GetScreenWidth() / 2 - MeasureText(title, 27) / 2, panelY + 22, 27, RAYWHITE);
        DrawText(subtitle, GetScreenWidth() / 2 - MeasureText(subtitle, 17) / 2, panelY + 68, 17, Color{ 180, 218, 230, 255 });
    }
};
