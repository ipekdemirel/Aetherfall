#include "WorldEnvironment.h"

#include <cmath>

#include <raylib.h>

namespace
{
    struct RuinPillar
    {
        Vector3 position;
        float height;
    };

    constexpr RuinPillar RUIN_PILLARS[] =
    {
        { Vector3{ -18.0f, 0.0f, -14.0f }, 4.8f },
        { Vector3{ -18.0f, 0.0f,   0.0f }, 3.3f },
        { Vector3{ -18.0f, 0.0f,  14.0f }, 5.4f },
        { Vector3{  18.0f, 0.0f, -14.0f }, 3.9f },
        { Vector3{  18.0f, 0.0f,   0.0f }, 5.1f },
        { Vector3{  18.0f, 0.0f,  14.0f }, 3.5f },
        { Vector3{ -10.0f, 0.0f, -19.0f }, 4.1f },
        { Vector3{  10.0f, 0.0f, -19.0f }, 5.0f },
        { Vector3{ -10.0f, 0.0f,  19.0f }, 3.7f },
        { Vector3{  10.0f, 0.0f,  19.0f }, 4.6f }
    };

    constexpr Vector3 BRAZIERS[] =
    {
        Vector3{ -6.0f, 0.0f, -5.0f },
        Vector3{  6.0f, 0.0f, -5.0f },
        Vector3{ -6.0f, 0.0f,  5.0f },
        Vector3{  6.0f, 0.0f,  5.0f }
    };

    constexpr Vector3 ALTAR_POSITIONS[] =
    {
        Vector3{ -10.0f, 0.0f, -8.0f },
        Vector3{  10.0f, 0.0f, -6.0f },
        Vector3{   0.0f, 0.0f, 10.0f }
    };

    Color TileColor(int x, int z, int currentLevel)
    {
        const int variation =
            ((x * 17 + z * 31) & 3) * 4;

        if (currentLevel == 1)
        {
            return Color{
                static_cast<unsigned char>(38 + variation),
                static_cast<unsigned char>(43 + variation),
                static_cast<unsigned char>(51 + variation),
                255
            };
        }

        return Color{
            static_cast<unsigned char>(44 + variation),
            static_cast<unsigned char>(35 + variation / 2),
            static_cast<unsigned char>(54 + variation),
            255
        };
    }

    void DrawRunicRing(
        Vector3 center,
        float radius,
        Color color,
        int dashOffset
    )
    {
        constexpr int SEGMENT_COUNT = 32;

        for (int segment = 0;
            segment < SEGMENT_COUNT;
            ++segment)
        {
            if ((segment + dashOffset) % 3 == 0)
            {
                continue;
            }

            const float firstAngle =
                2.0f * PI *
                segment /
                SEGMENT_COUNT;

            const float secondAngle =
                2.0f * PI *
                (segment + 1) /
                SEGMENT_COUNT;

            DrawLine3D(
                Vector3{
                    center.x + cosf(firstAngle) * radius,
                    center.y,
                    center.z + sinf(firstAngle) * radius
                },
                Vector3{
                    center.x + cosf(secondAngle) * radius,
                    center.y,
                    center.z + sinf(secondAngle) * radius
                },
                color
            );
        }
    }

    void DrawPillar(
        const RuinPillar& pillar,
        Color stoneColor
    )
    {
        const Vector3 base{
            pillar.position.x,
            0.18f,
            pillar.position.z
        };

        DrawCube(
            base,
            2.0f,
            0.36f,
            2.0f,
            Color{ 31, 34, 42, 255 }
        );

        DrawCubeWires(
            base,
            2.0f,
            0.36f,
            2.0f,
            Color{ 72, 78, 91, 255 }
        );

        const Vector3 columnCenter{
            pillar.position.x,
            0.40f + pillar.height * 0.5f,
            pillar.position.z
        };

        DrawCylinder(
            columnCenter,
            0.52f,
            0.68f,
            pillar.height,
            10,
            stoneColor
        );

        DrawCylinderWires(
            columnCenter,
            0.52f,
            0.68f,
            pillar.height,
            10,
            Color{ 79, 84, 98, 255 }
        );

        DrawCube(
            Vector3{
                columnCenter.x,
                0.42f + pillar.height,
                columnCenter.z
            },
            1.45f,
            0.30f,
            1.45f,
            Color{ 48, 52, 62, 255 }
        );
    }

    void DrawRubble(
        Vector3 center,
        Color stoneColor
    )
    {
        for (int piece = 0; piece < 5; ++piece)
        {
            const float angle =
                piece * 1.73f +
                center.x * 0.11f;

            const float distance =
                0.45f +
                piece * 0.16f;

            DrawCubeV(
                Vector3{
                    center.x + cosf(angle) * distance,
                    0.12f + (piece % 2) * 0.07f,
                    center.z + sinf(angle) * distance
                },
                Vector3{
                    0.38f + (piece % 3) * 0.12f,
                    0.22f + (piece % 2) * 0.14f,
                    0.34f + ((piece + 1) % 3) * 0.10f
                },
                stoneColor
            );
        }
    }

    void DrawBrazier(
        Vector3 position,
        float animationTime,
        Color energyColor
    )
    {
        DrawCylinder(
            Vector3{
                position.x,
                0.42f,
                position.z
            },
            0.32f,
            0.48f,
            0.84f,
            8,
            Color{ 43, 45, 54, 255 }
        );

        DrawCylinderWires(
            Vector3{
                position.x,
                0.42f,
                position.z
            },
            0.32f,
            0.48f,
            0.84f,
            8,
            Color{ 97, 103, 119, 255 }
        );

        const float pulse =
            0.5f +
            0.5f *
            sinf(
                animationTime * 6.0f +
                position.x
            );

        const Vector3 flamePosition{
            position.x,
            1.02f + pulse * 0.10f,
            position.z
        };

        DrawSphere(
            flamePosition,
            0.22f + pulse * 0.05f,
            energyColor
        );

        DrawSphere(
            Vector3{
                flamePosition.x,
                flamePosition.y + 0.23f,
                flamePosition.z
            },
            0.12f + pulse * 0.03f,
            Fade(RAYWHITE, 0.82f)
        );

        DrawSphere(
            flamePosition,
            0.55f + pulse * 0.08f,
            Fade(energyColor, 0.08f)
        );
    }
}

WorldEnvironment::WorldEnvironment()
    : animationTime(0.0f)
{
}

void WorldEnvironment::Update(float deltaTime)
{
    animationTime += deltaTime;
}

void WorldEnvironment::Draw(
    int currentLevel,
    bool bossFightActive
) const
{
    const Color energyColor =
        bossFightActive
        ? Color{ 255, 70, 108, 255 }
        : currentLevel == 1
        ? Color{ 74, 225, 255, 255 }
        : Color{ 180, 88, 255, 255 };

    const Color stoneColor =
        currentLevel == 1
        ? Color{ 54, 59, 69, 255 }
        : Color{ 57, 47, 67, 255 };

    DrawPlane(
        Vector3{ 0.0f, -0.16f, 0.0f },
        Vector2{ 72.0f, 72.0f },
        Color{ 12, 15, 22, 255 }
    );

    DrawPlane(
        Vector3{ 0.0f, -0.10f, 0.0f },
        Vector2{ 48.0f, 48.0f },
        currentLevel == 1
        ? Color{ 25, 30, 37, 255 }
        : Color{ 29, 23, 35, 255 }
    );

    for (int tileX = -11; tileX <= 11; ++tileX)
    {
        for (int tileZ = -11; tileZ <= 11; ++tileZ)
        {
            if (
                (tileX + tileZ * 2) % 17 == 0 ||
                (tileX * 3 - tileZ) % 23 == 0
                )
            {
                continue;
            }

            DrawCube(
                Vector3{
                    tileX * 2.0f,
                    -0.045f,
                    tileZ * 2.0f
                },
                1.88f,
                0.08f,
                1.88f,
                TileColor(
                    tileX,
                    tileZ,
                    currentLevel
                )
            );
        }
    }

    for (const RuinPillar& pillar : RUIN_PILLARS)
    {
        DrawPillar(
            pillar,
            stoneColor
        );

        DrawRubble(
            Vector3{
                pillar.position.x +
                    (pillar.position.z > 0.0f ? 1.1f : -0.8f),
                0.0f,
                pillar.position.z +
                    (pillar.position.x > 0.0f ? -0.7f : 0.9f)
            },
            Color{ 48, 51, 61, 255 }
        );
    }

    DrawRubble(
        Vector3{ -4.0f, 0.0f, -13.0f },
        stoneColor
    );

    DrawRubble(
        Vector3{ 7.5f, 0.0f, 13.0f },
        stoneColor
    );

    DrawRubble(
        Vector3{ -13.0f, 0.0f, 6.0f },
        stoneColor
    );

    for (const Vector3& brazier : BRAZIERS)
    {
        DrawBrazier(
            brazier,
            animationTime,
            energyColor
        );
    }

    const int ringOffset =
        static_cast<int>(
            animationTime * 4.0f
        );

    if (currentLevel == 1 && !bossFightActive)
    {
        for (const Vector3& altarPosition : ALTAR_POSITIONS)
        {
            DrawRunicRing(
                Vector3{
                    altarPosition.x,
                    0.025f,
                    altarPosition.z
                },
                2.45f,
                Fade(energyColor, 0.58f),
                ringOffset
            );
        }
    }
    else
    {
        DrawRunicRing(
            Vector3{ 0.0f, 0.025f, -10.0f },
            bossFightActive ? 8.0f : 4.6f,
            Fade(energyColor, 0.65f),
            ringOffset
        );

        DrawRunicRing(
            Vector3{ 0.0f, 0.026f, -10.0f },
            bossFightActive ? 6.7f : 3.4f,
            Fade(energyColor, 0.42f),
            -ringOffset
        );
    }

    DrawRunicRing(
        Vector3{ 0.0f, 0.022f, 0.0f },
        3.0f,
        Fade(energyColor, 0.24f),
        -ringOffset
    );
}
