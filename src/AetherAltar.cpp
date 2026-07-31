#include "AetherAltar.h"

#include <cmath>

#include <raymath.h>

AetherAltar::AetherAltar(Vector3 startPosition)
    : position(startPosition),
    activationProgress(0.0f),
    activationDuration(4.0f),
    interactionDistance(2.35f),
    effectTime(0.0f),
    activated(false),
    playerInRange(false)
{
}

bool AetherAltar::Update(
    float deltaTime,
    const Vector3& playerPosition,
    bool interactionHeld
)
{
    effectTime += deltaTime;

    if (activated)
    {
        playerInRange = false;
        return false;
    }

    Vector3 flatPlayerPosition =
        playerPosition;

    flatPlayerPosition.y =
        position.y;

    playerInRange =
        Vector3Distance(
            flatPlayerPosition,
            position
        ) <= interactionDistance;

    if (playerInRange && interactionHeld)
    {
        activationProgress +=
            deltaTime / activationDuration;

        if (activationProgress >= 1.0f)
        {
            activationProgress = 1.0f;
            activated = true;
            playerInRange = false;
            return true;
        }
    }
    else if (!playerInRange)
    {
        // Bölgeden ayrılınca ilerleme tamamen silinmez; yalnızca
        // yavaşça geriler. Böylece görev zorlayıcı ama adil kalır.
        activationProgress -=
            deltaTime * 0.05f;

        activationProgress =
            Clamp(
                activationProgress,
                0.0f,
                1.0f
            );
    }

    return false;
}

void AetherAltar::Draw() const
{
    const float pulse =
        0.5f +
        0.5f *
        sinf(effectTime * 4.0f);

    const Color energyColor =
        activated
        ? Color{ 92, 242, 255, 255 }
        : activationProgress > 0.0f
        ? Color{ 155, 96, 255, 255 }
    : Color{ 72, 58, 92, 255 };

    DrawCylinder(
        Vector3{
            position.x,
            position.y - 0.30f,
            position.z
        },
        1.35f,
        1.55f,
        0.45f,
        16,
        Color{ 42, 47, 58, 255 }
    );

    DrawCylinderWires(
        Vector3{
            position.x,
            position.y - 0.30f,
            position.z
        },
        1.35f,
        1.55f,
        0.45f,
        16,
        Color{ 104, 116, 135, 255 }
    );

    for (int i = 0; i < 4; i++)
    {
        const float angle =
            effectTime * 0.8f +
            i * PI / 2.0f;

        const Vector3 pillarPosition{
            position.x + cosf(angle) * 0.82f,
            position.y + 0.30f,
            position.z + sinf(angle) * 0.82f
        };

        DrawCube(
            pillarPosition,
            0.22f,
            1.25f,
            0.22f,
            Color{ 65, 70, 86, 255 }
        );

        DrawSphere(
            Vector3{
                pillarPosition.x,
                pillarPosition.y + 0.72f,
                pillarPosition.z
            },
            0.11f + pulse * 0.03f,
            energyColor
        );
    }

    const float crystalHeight =
        0.72f +
        sinf(effectTime * 2.8f) *
        0.10f;

    const Vector3 crystalPosition{
        position.x,
        position.y + crystalHeight,
        position.z
    };

    DrawSphere(
        crystalPosition,
        0.31f + pulse * 0.04f,
        energyColor
    );

    DrawSphereWires(
        crystalPosition,
        0.43f + pulse * 0.06f,
        8,
        8,
        Fade(energyColor, 0.70f)
    );

    if (!activated && activationProgress > 0.0f)
    {
        const int completedSegments =
            static_cast<int>(
                48.0f * activationProgress
                );

        for (int segment = 0;
            segment < completedSegments;
            segment++)
        {
            const float firstAngle =
                -PI / 2.0f +
                2.0f * PI *
                segment / 48.0f;

            const float secondAngle =
                -PI / 2.0f +
                2.0f * PI *
                (segment + 1) / 48.0f;

            DrawLine3D(
                Vector3{
                    position.x +
                    cosf(firstAngle) * 1.78f,
                    position.y + 0.03f,
                    position.z +
                    sinf(firstAngle) * 1.78f
                },
                Vector3{
                    position.x +
                    cosf(secondAngle) * 1.78f,
                    position.y + 0.03f,
                    position.z +
                    sinf(secondAngle) * 1.78f
                },
                energyColor
            );
        }
    }

    if (activated)
    {
        DrawCylinder(
            Vector3{
                position.x,
                position.y + 2.4f,
                position.z
            },
            0.08f,
            0.48f,
            3.4f,
            12,
            Fade(energyColor, 0.38f)
        );
    }
}

bool AetherAltar::IsActivated() const
{
    return activated;
}

bool AetherAltar::IsPlayerInRange() const
{
    return playerInRange;
}

float AetherAltar::GetActivationProgress() const
{
    return activationProgress;
}

Vector3 AetherAltar::GetPosition() const
{
    return position;
}