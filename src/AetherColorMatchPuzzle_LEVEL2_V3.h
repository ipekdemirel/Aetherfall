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
    {
        ResetState();
        BuildBoard();
    }

    void Start(Player& player)
    {
        ResetState();
        player.SetPosition(Vector3{ 0.0f, 1.0f, 8.7f });
        BuildBoard();
    }

    void Update(float deltaTime, Player& player)
    {
        if (completed)
        {
            return;
        }

        introTimer = MaxZero(introTimer - deltaTime);
        feedbackTimer = MaxZero(feedbackTimer - deltaTime);
        shuffleFlashTimer = MaxZero(shuffleFlashTimer - deltaTime);

        if (phase == Phase::MemoryPreview)
        {
            memoryPreviewTimer -= deltaTime;
            if (memoryPreviewTimer <= 0.0f)
            {
                phase = Phase::MemoryPlay;
                feedback = Feedback::MemoryGo;
                feedbackTimer = 1.5f;
            }
        }
        else if (phase == Phase::Rush)
        {
            rushTimer -= deltaTime;
            shuffleTimer -= deltaTime;

            if (shuffleTimer <= 0.0f)
            {
                ++boardGeneration;
                BuildBoard();
                shuffleTimer = 5.0f;
                shuffleFlashTimer = 0.65f;
            }

            if (rushTimer <= 0.0f)
            {
                rushHits = 0;
                rushTimer = RushTime;
                targetColorIndex = NextDifferentColor(targetColorIndex);
                ++boardGeneration;
                BuildBoard();
                feedback = Feedback::RushRestart;
                feedbackTimer = 2.3f;
            }
        }

        Vector3 playerPosition = player.GetPosition();
        playerPosition.y = 1.0f;
        playerPosition.x = Clamp(playerPosition.x, -10.2f, 10.2f);
        playerPosition.z = Clamp(
            playerPosition.z,
            doorOpen ? -13.8f : -9.1f,
            10.2f
        );
        player.SetPosition(playerPosition);

        ResolveOrbCollisions(player);
        playerPosition = player.GetPosition();
        nearbyOrbIndex = FindNearestOrb(playerPosition, InteractionRange);

        if (
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            phase != Phase::MemoryPreview &&
            !doorOpen
        )
        {
            if (nearbyOrbIndex >= 0)
            {
                TryHitOrb(nearbyOrbIndex);
            }
            else
            {
                feedback = Feedback::TooFar;
                feedbackTimer = 1.0f;
            }
        }

        // The old version used a circular distance check around the drawn gate.
        // Crossing the actual doorway plane is unambiguous and cannot leave the
        // player trapped in an already-open gate.
        playerPosition = player.GetPosition();
        if (
            doorOpen &&
            std::fabs(playerPosition.x) <= 2.0f &&
            playerPosition.z <= -10.75f
        )
        {
            completed = true;
        }
    }

    void DrawWorld() const
    {
        DrawPlane(
            Vector3{ 0.0f, 0.0f, 0.0f },
            Vector2{ 23.0f, 24.0f },
            Color{ 9, 19, 32, 255 }
        );

        for (int line = -10; line <= 10; line += 2)
        {
            DrawLine3D(
                Vector3{ static_cast<float>(line), 0.025f, -11.0f },
                Vector3{ static_cast<float>(line), 0.025f, 10.5f },
                Color{ 31, 62, 88, 160 }
            );
            DrawLine3D(
                Vector3{ -10.5f, 0.025f, static_cast<float>(line) },
                Vector3{ 10.5f, 0.025f, static_cast<float>(line) },
                Color{ 31, 62, 88, 160 }
            );
        }

        DrawArenaBorder();
        DrawPhaseCrystal();

        const float time = static_cast<float>(GetTime());
        for (int index = 0; index < OrbCount; ++index)
        {
            const Orb& orb = orbs[index];
            if (!orb.active)
            {
                continue;
            }

            const float bob = std::sin(time * 2.5f + index * 0.8f) * 0.12f;
            Vector3 position = orb.position;
            position.y += bob;
            const Color color = OrbColor(orb.colorIndex);

            DrawCylinder(
                Vector3{ position.x, 0.12f, position.z },
                0.82f,
                0.62f,
                0.20f,
                24,
                Fade(color, 0.30f)
            );
            DrawSphere(position, 0.64f, color);
            DrawSphereWires(position, 0.74f, 12, 12, Fade(RAYWHITE, 0.70f));
            DrawSphere(position, 0.20f, Fade(RAYWHITE, 0.58f));

            if (index == nearbyOrbIndex)
            {
                const bool correct = IsCorrectColor(orb.colorIndex);
                const Color highlight = correct ? LIME : ORANGE;
                DrawCylinder(
                    Vector3{ position.x, 0.07f, position.z },
                    1.12f,
                    0.94f,
                    0.09f,
                    28,
                    Fade(highlight, 0.74f)
                );
                DrawSphereWires(position, 0.94f, 14, 14, highlight);
            }
        }

        DrawDoor();
    }

    void DrawHUD() const
    {
        DrawText(
            "COLOR VAULT V3",
            20,
            205,
            16,
            Color{ 130, 255, 180, 255 }
        );

        const char* title = "AETHER COLOR VAULT: OVERLOAD";
        DrawText(
            title,
            GetScreenWidth() / 2 - MeasureText(title, 27) / 2,
            78,
            27,
            Color{ 112, 229, 255, 255 }
        );

        DrawPhaseHUD();

        if (!doorOpen && phase != Phase::MemoryPreview)
        {
            const char* interactionText;
            Color interactionColor;

            if (nearbyOrbIndex < 0)
            {
                interactionText = "MOVE BESIDE A BALL";
                interactionColor = Color{ 185, 216, 229, 255 };
            }
            else if (IsCorrectColor(orbs[nearbyOrbIndex].colorIndex))
            {
                interactionText = "CORRECT BALL IN RANGE - LEFT CLICK";
                interactionColor = LIME;
            }
            else
            {
                interactionText = "DECOY COLOR - DO NOT STRIKE";
                interactionColor = ORANGE;
            }

            DrawText(
                interactionText,
                GetScreenWidth() / 2 - MeasureText(interactionText, 19) / 2,
                184,
                19,
                interactionColor
            );
        }

        if (introTimer > 0.0f)
        {
            DrawPanel(
                "THREE VAULT TRIALS",
                "BUILD A CHAIN - REMEMBER THE CODE - SURVIVE THE RUSH"
            );
        }
        else if (feedbackTimer > 0.0f)
        {
            DrawFeedbackPanel();
        }

        if (shuffleFlashTimer > 0.0f)
        {
            const char* text = "AETHER SHIFT! BALLS REPOSITIONED";
            DrawText(
                text,
                GetScreenWidth() / 2 - MeasureText(text, 24) / 2,
                220,
                24,
                Color{ 255, 209, 74, 255 }
            );
        }

        const char* controls =
            "WASD: MOVE   LEFT SHIFT: RUN   SPACE: DASH   LEFT MOUSE: STRIKE";
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
        return completedTrials;
    }

    static int GetRequiredMatchCount()
    {
        return RequiredTrials;
    }

private:
    static constexpr int OrbCount = 16;
    static constexpr int ColorCount = 4;
    static constexpr int RequiredTrials = 3;
    static constexpr int ChainGoal = 6;
    static constexpr int MemoryLength = 6;
    static constexpr int RushGoal = 10;
    static constexpr float RushTime = 38.0f;
    static constexpr float InteractionRange = 2.85f;
    static constexpr float CollisionRadius = 1.30f;

    enum class Phase
    {
        Chain,
        MemoryPreview,
        MemoryPlay,
        Rush,
        GateOpen
    };

    enum class Feedback
    {
        None,
        Correct,
        Wrong,
        TrialComplete,
        MemoryGo,
        MemoryWrong,
        RushRestart,
        GateOpened,
        TooFar
    };

    struct Orb
    {
        Vector3 position;
        int colorIndex;
        bool active;
    };

    std::array<Orb, OrbCount> orbs{};
    std::array<int, MemoryLength> memorySequence{};
    Phase phase = Phase::Chain;
    Feedback feedback = Feedback::None;
    int targetColorIndex = 0;
    int chainHits = 0;
    int memoryIndex = 0;
    int rushHits = 0;
    int completedTrials = 0;
    int boardGeneration = 0;
    int nearbyOrbIndex = -1;
    bool doorOpen = false;
    bool completed = false;
    float introTimer = 7.0f;
    float feedbackTimer = 0.0f;
    float memoryPreviewTimer = 0.0f;
    float rushTimer = RushTime;
    float shuffleTimer = 5.0f;
    float shuffleFlashTimer = 0.0f;

    void ResetState()
    {
        phase = Phase::Chain;
        feedback = Feedback::None;
        targetColorIndex = 2;
        chainHits = 0;
        memoryIndex = 0;
        rushHits = 0;
        completedTrials = 0;
        boardGeneration = 0;
        nearbyOrbIndex = -1;
        doorOpen = false;
        completed = false;
        introTimer = 7.0f;
        feedbackTimer = 0.0f;
        memoryPreviewTimer = 0.0f;
        rushTimer = RushTime;
        shuffleTimer = 5.0f;
        shuffleFlashTimer = 0.0f;
    }

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
        static const char* names[ColorCount]{ "RED", "BLUE", "YELLOW", "GREEN" };
        return names[colorIndex % ColorCount];
    }

    int NextDifferentColor(int current) const
    {
        return (current + 1 + (boardGeneration % 3)) % ColorCount;
    }

    bool IsCorrectColor(int colorIndex) const
    {
        if (phase == Phase::MemoryPlay)
        {
            return colorIndex == memorySequence[memoryIndex];
        }
        return colorIndex == targetColorIndex;
    }

    void BuildBoard()
    {
        int index = 0;
        for (int row = 0; row < 4; ++row)
        {
            for (int column = 0; column < 4; ++column)
            {
                const int shiftedColumn = (column + boardGeneration + row) % 4;
                const int color =
                    (row * 2 + column + boardGeneration) % ColorCount;

                orbs[index] = Orb{
                    Vector3{
                        -7.5f + shiftedColumn * 5.0f,
                        0.92f,
                        -6.2f + row * 4.5f
                    },
                    color,
                    true
                };
                ++index;
            }
        }
    }

    void BuildMemorySequence()
    {
        const int pattern[MemoryLength]{ 0, 2, 1, 3, 2, 0 };
        for (int i = 0; i < MemoryLength; ++i)
        {
            memorySequence[i] = (pattern[i] + boardGeneration) % ColorCount;
        }
    }

    int FindNearestOrb(Vector3 playerPosition, float maximumDistance) const
    {
        int nearestIndex = -1;
        float nearestDistance = maximumDistance;

        for (int index = 0; index < OrbCount; ++index)
        {
            if (!orbs[index].active)
            {
                continue;
            }

            const float distance = HorizontalDistance(orbs[index].position, playerPosition);
            if (distance < nearestDistance)
            {
                nearestIndex = index;
                nearestDistance = distance;
            }
        }
        return nearestIndex;
    }

    void ResolveOrbCollisions(Player& player) const
    {
        Vector3 playerPosition = player.GetPosition();
        for (int pass = 0; pass < 2; ++pass)
        {
            for (const Orb& orb : orbs)
            {
                if (!orb.active)
                {
                    continue;
                }

                Vector3 away = Vector3Subtract(playerPosition, orb.position);
                away.y = 0.0f;
                float distance = Vector3Length(away);
                if (distance >= CollisionRadius)
                {
                    continue;
                }

                if (distance < 0.001f)
                {
                    away = Vector3{ 0.0f, 0.0f, 1.0f };
                    distance = 0.0f;
                }
                else
                {
                    away = Vector3Scale(away, 1.0f / distance);
                }

                playerPosition = Vector3Add(
                    playerPosition,
                    Vector3Scale(away, CollisionRadius - distance)
                );
            }
        }

        playerPosition.x = Clamp(playerPosition.x, -10.2f, 10.2f);
        playerPosition.z = Clamp(playerPosition.z, doorOpen ? -13.8f : -9.1f, 10.2f);
        playerPosition.y = 1.0f;
        player.SetPosition(playerPosition);
    }

    void TryHitOrb(int index)
    {
        if (index < 0 || index >= OrbCount || !orbs[index].active)
        {
            return;
        }

        if (!IsCorrectColor(orbs[index].colorIndex))
        {
            feedback = phase == Phase::MemoryPlay
                ? Feedback::MemoryWrong
                : Feedback::Wrong;
            feedbackTimer = 1.25f;

            if (phase == Phase::Chain)
            {
                chainHits = chainHits > 0 ? chainHits - 1 : 0;
            }
            else if (phase == Phase::MemoryPlay)
            {
                memoryIndex = 0;
                ReactivateAllOrbs();
            }
            else if (phase == Phase::Rush)
            {
                rushTimer = MaxZero(rushTimer - 3.0f);
            }
            return;
        }

        orbs[index].active = false;
        feedback = Feedback::Correct;
        feedbackTimer = 0.65f;

        if (phase == Phase::Chain)
        {
            ++chainHits;
            targetColorIndex = NextDifferentColor(targetColorIndex);
            ++boardGeneration;
            BuildBoard();

            if (chainHits >= ChainGoal)
            {
                completedTrials = 1;
                phase = Phase::MemoryPreview;
                memoryPreviewTimer = 5.5f;
                memoryIndex = 0;
                BuildMemorySequence();
                feedback = Feedback::TrialComplete;
                feedbackTimer = 2.0f;
            }
        }
        else if (phase == Phase::MemoryPlay)
        {
            ++memoryIndex;
            if (memoryIndex >= MemoryLength)
            {
                completedTrials = 2;
                phase = Phase::Rush;
                rushHits = 0;
                rushTimer = RushTime;
                shuffleTimer = 5.0f;
                targetColorIndex = 1;
                ++boardGeneration;
                BuildBoard();
                feedback = Feedback::TrialComplete;
                feedbackTimer = 2.0f;
            }
        }
        else if (phase == Phase::Rush)
        {
            ++rushHits;
            targetColorIndex = NextDifferentColor(targetColorIndex);

            if (rushHits % 2 == 0)
            {
                ++boardGeneration;
                BuildBoard();
                shuffleFlashTimer = 0.45f;
            }

            if (rushHits >= RushGoal)
            {
                completedTrials = 3;
                phase = Phase::GateOpen;
                doorOpen = true;
                nearbyOrbIndex = -1;
                feedback = Feedback::GateOpened;
                feedbackTimer = 4.0f;
            }
        }
    }

    void ReactivateAllOrbs()
    {
        for (Orb& orb : orbs)
        {
            orb.active = true;
        }
    }

    void DrawPhaseHUD() const
    {
        if (phase == Phase::Chain)
        {
            const char* text = TextFormat(
                "TRIAL 1/3 - CHAIN: %d/%d    TARGET: %s",
                chainHits,
                ChainGoal,
                ColorName(targetColorIndex)
            );
            DrawCentered(text, 116, 22, OrbColor(targetColorIndex));
            DrawCentered("EACH CORRECT HIT CHANGES THE TARGET COLOR", 150, 18, RAYWHITE);
        }
        else if (phase == Phase::MemoryPreview)
        {
            DrawCentered("TRIAL 2/3 - MEMORIZE THIS COLOR CODE", 116, 22, SKYBLUE);
            DrawMemorySequence(true);
        }
        else if (phase == Phase::MemoryPlay)
        {
            const char* text = TextFormat(
                "TRIAL 2/3 - REPEAT THE CODE: %d/%d",
                memoryIndex,
                MemoryLength
            );
            DrawCentered(text, 116, 22, SKYBLUE);
            DrawCentered("THE CODE IS HIDDEN - A WRONG COLOR RESTARTS THIS TRIAL", 150, 18, RAYWHITE);
        }
        else if (phase == Phase::Rush)
        {
            const char* text = TextFormat(
                "TRIAL 3/3 - AETHER RUSH: %d/%d    TIME: %.1f    TARGET: %s",
                rushHits,
                RushGoal,
                rushTimer,
                ColorName(targetColorIndex)
            );
            DrawCentered(text, 116, 22, OrbColor(targetColorIndex));
            DrawCentered("BALLS SHIFT EVERY 5 SECONDS - WRONG HIT COSTS 3 SECONDS", 150, 18, RAYWHITE);
        }
        else
        {
            DrawCentered("ALL TRIALS CLEARED", 116, 24, GREEN);
            DrawCentered("THE NORTH GATE IS OPEN - WALK THROUGH THE GREEN LIGHT", 151, 19, LIME);
        }
    }

    void DrawMemorySequence(bool reveal) const
    {
        const int radius = 20;
        const int spacing = 58;
        const int totalWidth = (MemoryLength - 1) * spacing;
        const int startX = GetScreenWidth() / 2 - totalWidth / 2;

        for (int i = 0; i < MemoryLength; ++i)
        {
            const Color color = reveal
                ? OrbColor(memorySequence[i])
                : Color{ 70, 80, 95, 255 };
            DrawCircle(startX + i * spacing, 157, radius, color);
            DrawCircleLines(startX + i * spacing, 157, radius, RAYWHITE);
        }
    }

    static void DrawCentered(const char* text, int y, int size, Color color)
    {
        DrawText(text, GetScreenWidth() / 2 - MeasureText(text, size) / 2, y, size, color);
    }

    void DrawFeedbackPanel() const
    {
        switch (feedback)
        {
        case Feedback::Correct:
            DrawPanel("RESONANCE HIT", "KEEP MOVING - FOLLOW THE NEXT COLOR");
            break;
        case Feedback::Wrong:
            DrawPanel("DECOY STRUCK", "CHAIN REDUCED - CHECK THE TARGET ABOVE");
            break;
        case Feedback::TrialComplete:
            DrawPanel("TRIAL COMPLETE", "THE VAULT IS CHANGING ITS RULES");
            break;
        case Feedback::MemoryGo:
            DrawPanel("CODE HIDDEN", "REPEAT THE SIX COLORS FROM MEMORY");
            break;
        case Feedback::MemoryWrong:
            DrawPanel("MEMORY BROKEN", "THE CODE PROGRESS RESTARTED - TRY AGAIN");
            break;
        case Feedback::RushRestart:
            DrawPanel("TIME FRACTURED", "THE FINAL RUSH RESTARTED - YOU KEEP BOTH CLEARED TRIALS");
            break;
        case Feedback::GateOpened:
            DrawPanel("VAULT OVERLOADED", "RUN THROUGH THE OPEN NORTH GATE");
            break;
        case Feedback::TooFar:
            DrawPanel("TOO FAR", "MOVE BESIDE A BALL UNTIL ITS RING APPEARS");
            break;
        default:
            break;
        }
    }

    void DrawArenaBorder() const
    {
        const Color wall{ 19, 43, 65, 255 };
        const Color wire{ 59, 193, 226, 220 };

        DrawCube(Vector3{ -11.0f, 1.0f, 0.0f }, 0.5f, 2.0f, 22.0f, wall);
        DrawCubeWires(Vector3{ -11.0f, 1.0f, 0.0f }, 0.5f, 2.0f, 22.0f, wire);
        DrawCube(Vector3{ 11.0f, 1.0f, 0.0f }, 0.5f, 2.0f, 22.0f, wall);
        DrawCubeWires(Vector3{ 11.0f, 1.0f, 0.0f }, 0.5f, 2.0f, 22.0f, wire);
        DrawCube(Vector3{ 0.0f, 1.0f, 11.0f }, 22.0f, 2.0f, 0.5f, wall);
        DrawCubeWires(Vector3{ 0.0f, 1.0f, 11.0f }, 22.0f, 2.0f, 0.5f, wire);

        DrawCube(Vector3{ -6.8f, 1.0f, -11.0f }, 8.0f, 2.0f, 0.5f, wall);
        DrawCube(Vector3{ 6.8f, 1.0f, -11.0f }, 8.0f, 2.0f, 0.5f, wall);
    }

    void DrawPhaseCrystal() const
    {
        const Color color = phase == Phase::GateOpen
            ? LIME
            : phase == Phase::MemoryPreview || phase == Phase::MemoryPlay
            ? SKYBLUE
            : OrbColor(targetColorIndex);

        DrawCylinder(Vector3{ 0.0f, 0.12f, 1.0f }, 1.25f, 1.0f, 0.22f, 32, Fade(color, 0.28f));
        DrawSphere(Vector3{ 0.0f, 0.56f, 1.0f }, 0.30f, color);
        DrawSphereWires(Vector3{ 0.0f, 0.56f, 1.0f }, 0.49f, 10, 10, Fade(RAYWHITE, 0.80f));
    }

    void DrawDoor() const
    {
        const Vector3 door{ 0.0f, 1.5f, -10.9f };
        const Color frame = doorOpen
            ? Color{ 76, 235, 141, 255 }
            : Color{ 48, 69, 86, 255 };

        DrawCube(Vector3{ -2.0f, 1.8f, door.z }, 0.55f, 3.6f, 0.65f, frame);
        DrawCube(Vector3{ 2.0f, 1.8f, door.z }, 0.55f, 3.6f, 0.65f, frame);
        DrawCube(Vector3{ 0.0f, 3.55f, door.z }, 4.55f, 0.45f, 0.65f, frame);

        if (doorOpen)
        {
            DrawCube(Vector3{ 0.0f, 1.75f, door.z }, 3.3f, 3.0f, 0.05f, Fade(LIME, 0.22f));
            DrawCylinder(Vector3{ 0.0f, 0.08f, -11.8f }, 1.8f, 1.35f, 0.10f, 28, Fade(LIME, 0.55f));
        }
        else
        {
            DrawCube(Vector3{ 0.0f, 1.7f, door.z }, 3.35f, 3.0f, 0.35f, Color{ 23, 34, 48, 255 });
            DrawCubeWires(Vector3{ 0.0f, 1.7f, door.z }, 3.35f, 3.0f, 0.35f, Color{ 101, 125, 146, 255 });
        }
    }

    static void DrawPanel(const char* title, const char* subtitle)
    {
        const int width = 700;
        const int height = 116;
        const int x = GetScreenWidth() / 2 - width / 2;
        const int y = GetScreenHeight() / 2 - height / 2;

        DrawRectangle(x, y, width, height, Color{ 5, 14, 23, 228 });
        DrawRectangleLinesEx(
            Rectangle{
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(width),
                static_cast<float>(height)
            },
            2.0f,
            Color{ 84, 218, 245, 255 }
        );
        DrawCentered(title, y + 22, 27, RAYWHITE);
        DrawCentered(subtitle, y + 69, 17, Color{ 180, 218, 230, 255 });
    }
};
