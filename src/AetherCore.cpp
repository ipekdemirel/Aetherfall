#include "AetherCore.h"

#include <cmath>

#include <raymath.h>
#include <rlgl.h>

AetherCore::AetherCore()
    :
    position{ 0.0f, 1.2f, 0.0f },
    rotationAngle(0.0f),
    bobTimer(0.0f),
    collectionRange(1.8f),
    spawned(false),
    collected(false)
{
}

void AetherCore::Spawn(Vector3 spawnPosition)
{
    position = {
        spawnPosition.x,
        1.2f,
        spawnPosition.z
    };

    rotationAngle = 0.0f;
    bobTimer = 0.0f;

    spawned = true;
    collected = false;
}

void AetherCore::Update(
    float deltaTime,
    Vector3 playerPosition
)
{
    if (!spawned || collected)
    {
        return;
    }

    rotationAngle += 90.0f * deltaTime;

    if (rotationAngle >= 360.0f)
    {
        rotationAngle -= 360.0f;
    }

    bobTimer += deltaTime * 2.5f;

    Vector3 coreToPlayer =
        Vector3Subtract(
            playerPosition,
            position
        );

    coreToPlayer.y = 0.0f;

    if (
        Vector3Length(coreToPlayer) <=
        collectionRange
        )
    {
        collected = true;
        spawned = false;
    }
}

void AetherCore::Draw() const
{
    if (!spawned || collected)
    {
        return;
    }

    const float bobOffset =
        sinf(bobTimer) * 0.20f;

    const Vector3 drawPosition{
        position.x,
        position.y + bobOffset,
        position.z
    };

    // Core'un alınabileceği alanı gösterir.
    DrawCircle3D(
        Vector3{
            position.x,
            0.03f,
            position.z
        },
        collectionRange,
        Vector3{ 1.0f, 0.0f, 0.0f },
        90.0f,
        Fade(SKYBLUE, 0.22f)
    );

    // Core'un ortasındaki parlak küre.
    DrawSphere(
        drawPosition,
        0.72f,
        Color{
            60,
            220,
            255,
            255
        }
    );

    // Parlak kürenin dış çizgileri.
    DrawSphereWires(
        drawPosition,
        0.82f,
        12,
        12,
        WHITE
    );

    // Dönen dış küp.
    // DrawCubePro raylib 5.5'te bulunmadığı için
    // dönüş işlemini rlgl ile gerçekleştiriyoruz.
    rlPushMatrix();

    rlTranslatef(
        drawPosition.x,
        drawPosition.y,
        drawPosition.z
    );

    rlRotatef(
        rotationAngle,
        0.0f,
        1.0f,
        0.0f
    );

    DrawCube(
        Vector3{ 0.0f, 0.0f, 0.0f },
        1.10f,
        1.10f,
        1.10f,
        Fade(BLUE, 0.75f)
    );

    DrawCubeWires(
        Vector3{ 0.0f, 0.0f, 0.0f },
        1.10f,
        1.10f,
        1.10f,
        WHITE
    );

    rlPopMatrix();
}

void AetherCore::Reset()
{
    position = {
        0.0f,
        1.2f,
        0.0f
    };

    rotationAngle = 0.0f;
    bobTimer = 0.0f;

    spawned = false;
    collected = false;
}

bool AetherCore::IsSpawned() const
{
    return spawned;
}

bool AetherCore::IsCollected() const
{
    return collected;
}