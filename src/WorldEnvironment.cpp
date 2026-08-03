#include <raylib.h>
#include <raymath.h>

#include "WorldEnvironment.h"

#include <cmath>

namespace
{
    struct SceneObject
    {
        Vector3 position;
        float scale;
    };

    constexpr SceneObject RUIN_PILLARS[] =
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

    constexpr Vector3 ALTAR_POSITIONS[] =
    {
        Vector3{ -10.0f, 0.0f, -8.0f },
        Vector3{  10.0f, 0.0f, -6.0f },
        Vector3{   0.0f, 0.0f, 10.0f }
    };

    constexpr SceneObject FOREST_TREES[] =
    {
        { Vector3{ -20.0f, 0.0f, -18.0f }, 1.20f },
        { Vector3{ -14.0f, 0.0f, -20.0f }, 0.92f },
        { Vector3{  -7.0f, 0.0f, -21.0f }, 1.15f },
        { Vector3{   7.0f, 0.0f, -21.0f }, 1.00f },
        { Vector3{  15.0f, 0.0f, -20.0f }, 1.22f },
        { Vector3{  21.0f, 0.0f, -15.0f }, 0.96f },
        { Vector3{ -21.0f, 0.0f,  -9.0f }, 1.05f },
        { Vector3{  21.0f, 0.0f,  -5.0f }, 1.18f },
        { Vector3{ -21.0f, 0.0f,   2.0f }, 1.24f },
        { Vector3{  21.0f, 0.0f,   7.0f }, 0.90f },
        { Vector3{ -20.0f, 0.0f,  14.0f }, 1.12f },
        { Vector3{ -12.0f, 0.0f,  20.0f }, 0.95f },
        { Vector3{  -3.0f, 0.0f,  21.0f }, 1.20f },
        { Vector3{   7.0f, 0.0f,  20.0f }, 1.05f },
        { Vector3{  16.0f, 0.0f,  19.0f }, 1.24f },
        { Vector3{ -13.0f, 0.0f,   8.0f }, 0.72f },
        { Vector3{  13.0f, 0.0f,  10.0f }, 0.76f },
        { Vector3{ -14.0f, 0.0f,  -7.0f }, 0.68f },
        { Vector3{  14.0f, 0.0f,  -9.0f }, 0.74f }
    };

    constexpr SceneObject LAVA_POOLS[] =
    {
        { Vector3{ -8.0f, 0.0f,   2.0f }, 2.6f },
        { Vector3{  8.0f, 0.0f,   1.0f }, 2.5f },
        { Vector3{-10.0f, 0.0f, -10.0f }, 2.6f },
        { Vector3{ 10.0f, 0.0f,  -9.0f }, 2.7f },
        { Vector3{  0.0f, 0.0f,   8.0f }, 2.6f }
    };

    void DrawRing(
        Vector3 center,
        float radius,
        Color color,
        int dashOffset
    )
    {
        constexpr int SEGMENTS = 36;

        for (int i = 0; i < SEGMENTS; ++i)
        {
            if ((i + dashOffset) % 4 == 0)
            {
                continue;
            }

            const float first = 2.0f * PI * i / SEGMENTS;
            const float second = 2.0f * PI * (i + 1) / SEGMENTS;

            DrawLine3D(
                Vector3{
                    center.x + cosf(first) * radius,
                    center.y,
                    center.z + sinf(first) * radius
                },
                Vector3{
                    center.x + cosf(second) * radius,
                    center.y,
                    center.z + sinf(second) * radius
                },
                color
            );
        }
    }

    void DrawRuinPillar(const SceneObject& pillar)
    {
        DrawCube(
            Vector3{ pillar.position.x, 0.18f, pillar.position.z },
            2.0f, 0.36f, 2.0f,
            Color{ 31, 34, 42, 255 }
        );

        DrawCylinder(
            Vector3{
                pillar.position.x,
                0.40f + pillar.scale * 0.5f,
                pillar.position.z
            },
            0.52f, 0.68f, pillar.scale, 10,
            Color{ 54, 59, 69, 255 }
        );

        DrawCube(
            Vector3{
                pillar.position.x,
                0.42f + pillar.scale,
                pillar.position.z
            },
            1.45f, 0.30f, 1.45f,
            Color{ 48, 52, 62, 255 }
        );
    }

    void DrawRubble(Vector3 center, Color color)
    {
        for (int piece = 0; piece < 5; ++piece)
        {
            const float angle = piece * 1.73f + center.x * 0.11f;
            const float distance = 0.45f + piece * 0.16f;

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
                color
            );
        }
    }

    void DrawRuins(float time)
    {
        DrawPlane(
            Vector3{ 0.0f, -0.12f, 0.0f },
            Vector2{ 72.0f, 72.0f },
            Color{ 12, 15, 22, 255 }
        );

        DrawPlane(
            Vector3{ 0.0f, -0.08f, 0.0f },
            Vector2{ 48.0f, 48.0f },
            Color{ 25, 30, 37, 255 }
        );

        for (int x = -11; x <= 11; ++x)
        {
            for (int z = -11; z <= 11; ++z)
            {
                if ((x + z * 2) % 17 == 0)
                {
                    continue;
                }

                const int variation = ((x * 17 + z * 31) & 3) * 4;

                DrawCube(
                    Vector3{ x * 2.0f, -0.035f, z * 2.0f },
                    1.88f, 0.08f, 1.88f,
                    Color{
                        static_cast<unsigned char>(38 + variation),
                        static_cast<unsigned char>(43 + variation),
                        static_cast<unsigned char>(51 + variation),
                        255
                    }
                );
            }
        }

        for (const SceneObject& pillar : RUIN_PILLARS)
        {
            DrawRuinPillar(pillar);
            DrawRubble(
                Vector3{
                    pillar.position.x + 0.8f,
                    0.0f,
                    pillar.position.z - 0.7f
                },
                Color{ 48, 51, 61, 255 }
            );
        }

        const int offset = static_cast<int>(time * 4.0f);

        for (const Vector3& altar : ALTAR_POSITIONS)
        {
            DrawRing(
                Vector3{ altar.x, 0.025f, altar.z },
                2.45f,
                Fade(Color{ 74, 225, 255, 255 }, 0.58f),
                offset
            );
        }
    }

    void DrawTree(const SceneObject& tree, float time, int index)
    {
        const float trunkHeight = 5.2f * tree.scale;
        const float sway = sinf(time * 0.9f + index * 1.7f) * 0.10f;

        DrawCylinder(
            Vector3{
                tree.position.x,
                trunkHeight * 0.5f,
                tree.position.z
            },
            0.55f * tree.scale,
            0.82f * tree.scale,
            trunkHeight,
            9,
            Color{ 63, 42, 34, 255 }
        );

        DrawSphere(
            Vector3{
                tree.position.x + sway,
                trunkHeight + 0.25f,
                tree.position.z
            },
            2.15f * tree.scale,
            Color{ 28, 83, 58, 255 }
        );

        DrawSphere(
            Vector3{
                tree.position.x - 0.9f * tree.scale,
                trunkHeight - 0.15f,
                tree.position.z + 0.35f
            },
            1.45f * tree.scale,
            Color{ 36, 111, 67, 255 }
        );

        DrawSphere(
            Vector3{
                tree.position.x + 0.9f * tree.scale,
                trunkHeight + 0.10f,
                tree.position.z - 0.25f
            },
            1.55f * tree.scale,
            Color{ 20, 67, 52, 255 }
        );
    }

    void DrawCorruptedForest(float time)
    {
        DrawPlane(
            Vector3{ 0.0f, -0.12f, 0.0f },
            Vector2{ 72.0f, 72.0f },
            Color{ 8, 20, 17, 255 }
        );

        DrawPlane(
            Vector3{ 0.0f, -0.08f, 0.0f },
            Vector2{ 48.0f, 48.0f },
            Color{ 23, 55, 38, 255 }
        );

        for (int x = -11; x <= 11; ++x)
        {
            for (int z = -11; z <= 11; ++z)
            {
                const bool path =
                    std::abs(x) <= 2 ||
                    std::abs(z + 2) <= 1;

                const Color groundColor = path
                    ? Color{ 62, 56, 43, 255 }
                    : ((x + z) & 1)
                    ? Color{ 28, 69, 43, 255 }
                    : Color{ 32, 78, 48, 255 };

                DrawCube(
                    Vector3{ x * 2.0f, -0.035f, z * 2.0f },
                    1.92f, 0.08f, 1.92f,
                    groundColor
                );
            }
        }

        int index = 0;
        for (const SceneObject& tree : FOREST_TREES)
        {
            DrawTree(tree, time, index++);
        }

        for (int mushroom = 0; mushroom < 18; ++mushroom)
        {
            const float x = -18.0f + static_cast<float>((mushroom * 37) % 36);
            const float z = -18.0f + static_cast<float>((mushroom * 23) % 36);
            const float pulse = 0.5f + 0.5f * sinf(time * 3.0f + mushroom);

            DrawCylinder(
                Vector3{ x, 0.18f, z },
                0.07f, 0.09f, 0.36f, 6,
                Color{ 204, 220, 183, 255 }
            );

            DrawSphere(
                Vector3{ x, 0.40f, z },
                0.18f + pulse * 0.04f,
                mushroom % 3 == 0
                ? Color{ 169, 79, 255, 255 }
                : Color{ 71, 222, 184, 255 }
            );
        }

        const int offset = static_cast<int>(time * 5.0f);
        DrawRing(
            Vector3{ 0.0f, 0.025f, -4.0f },
            4.7f,
            Fade(Color{ 67, 225, 255, 255 }, 0.55f),
            offset
        );

        DrawRing(
            Vector3{ 0.0f, 0.026f, -4.0f },
            3.4f,
            Fade(Color{ 169, 79, 255, 255 }, 0.42f),
            -offset
        );
    }

    void DrawStalagmite(Vector3 position, float height)
    {
        DrawCylinderEx(
            Vector3{ position.x, -0.05f, position.z },
            Vector3{ position.x, height, position.z },
            0.95f,
            0.08f,
            8,
            Color{ 42, 35, 40, 255 }
        );
    }

    void DrawLavaCavern(float time)
    {
        DrawPlane(
            Vector3{ 0.0f, -0.14f, 0.0f },
            Vector2{ 72.0f, 72.0f },
            Color{ 10, 7, 9, 255 }
        );

        DrawPlane(
            Vector3{ 0.0f, -0.09f, 0.0f },
            Vector2{ 48.0f, 48.0f },
            Color{ 39, 31, 34, 255 }
        );

        for (int x = -11; x <= 11; ++x)
        {
            for (int z = -11; z <= 11; ++z)
            {
                const int variation = std::abs(x * 11 + z * 7) % 4;
                DrawCube(
                    Vector3{ x * 2.0f, -0.035f, z * 2.0f },
                    1.90f, 0.08f, 1.90f,
                    Color{
                        static_cast<unsigned char>(45 + variation * 4),
                        static_cast<unsigned char>(34 + variation * 2),
                        static_cast<unsigned char>(37 + variation * 2),
                        255
                    }
                );
            }
        }

        for (const SceneObject& pool : LAVA_POOLS)
        {
            const float pulse = 0.5f + 0.5f * sinf(time * 3.2f + pool.position.x);

            DrawCircle3D(
                Vector3{ pool.position.x, 0.012f, pool.position.z },
                pool.scale + pulse * 0.16f,
                Vector3{ 1.0f, 0.0f, 0.0f },
                90.0f,
                Color{ 118, 25, 12, 255 }
            );

            DrawCircle3D(
                Vector3{ pool.position.x, 0.024f, pool.position.z },
                pool.scale * 0.78f + pulse * 0.12f,
                Vector3{ 1.0f, 0.0f, 0.0f },
                90.0f,
                Color{ 255, 72, 15, 255 }
            );

            DrawCircle3D(
                Vector3{ pool.position.x, 0.030f, pool.position.z },
                pool.scale * 0.42f + pulse * 0.10f,
                Vector3{ 1.0f, 0.0f, 0.0f },
                90.0f,
                Color{ 255, 193, 52, 255 }
            );
        }

        for (int i = 0; i < 16; ++i)
        {
            const float angle = 2.0f * PI * i / 16.0f;
            const float radius = 21.0f + (i % 3) * 1.3f;
            DrawStalagmite(
                Vector3{
                    cosf(angle) * radius,
                    0.0f,
                    sinf(angle) * radius
                },
                3.5f + (i % 5) * 0.8f
            );
        }

        const int offset = static_cast<int>(time * 7.0f);
        DrawRing(
            Vector3{ 0.0f, 0.035f, -10.0f },
            8.0f,
            Fade(Color{ 255, 76, 25, 255 }, 0.72f),
            offset
        );
        DrawRing(
            Vector3{ 0.0f, 0.037f, -10.0f },
            6.7f,
            Fade(GOLD, 0.42f),
            -offset
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
    (void)bossFightActive;

    if (currentLevel == 1)
    {
        DrawRuins(animationTime);
    }
    else if (currentLevel == 2)
    {
        DrawCorruptedForest(animationTime);
    }
    else if (currentLevel == 3)
    {
        DrawLavaCavern(animationTime);
    }
    else
    {
        DrawRuins(animationTime);
    }
}

bool WorldEnvironment::IsHazardAt(
    int currentLevel,
    Vector3 position
) const
{
    if (currentLevel != 3)
    {
        return false;
    }

    position.y = 0.0f;

    for (const SceneObject& pool : LAVA_POOLS)
    {
        Vector3 toPool = Vector3Subtract(position, pool.position);
        toPool.y = 0.0f;

        if (Vector3Length(toPool) <= pool.scale * 0.82f)
        {
            return true;
        }
    }

    return false;
}
