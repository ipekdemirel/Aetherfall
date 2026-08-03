#include "DefenseCore.h"

#include "Enemy.h"

#include <algorithm>
#include <cmath>

#include <raymath.h>
#include <rlgl.h>

DefenseCore::DefenseCore()
    : position{ 0.0f, 1.25f, -4.0f },
    health(500.0f),
    maxHealth(500.0f),
    enemyAttackRange(2.35f),
    damagePerEnemy(7.0f),
    damageInterval(1.0f),
    damageTimer(0.0f),
    protectionTimer(6.0f),
    maxSimultaneousAttackers(1),
    activeAttackerCount(0),
    visualTime(0.0f),
    hitFlashTimer(0.0f)
{
}

void DefenseCore::Reset(Vector3 startPosition)
{
    position = startPosition;
    position.y = 1.25f;

    health = maxHealth;
    damageTimer = 0.0f;
    protectionTimer = 6.0f;
    activeAttackerCount = 0;
    visualTime = 0.0f;
    hitFlashTimer = 0.0f;
}

bool DefenseCore::Update(
    float deltaTime,
    const std::vector<Enemy>& enemies
)
{
    visualTime += deltaTime;

    if (protectionTimer > 0.0f)
    {
        protectionTimer -= deltaTime;

        if (protectionTimer < 0.0f)
        {
            protectionTimer = 0.0f;
        }
    }

    if (hitFlashTimer > 0.0f)
    {
        hitFlashTimer -= deltaTime;

        if (hitFlashTimer < 0.0f)
        {
            hitFlashTimer = 0.0f;
        }
    }

    if (!IsAlive())
    {
        activeAttackerCount = 0;
        return false;
    }

    int attackingEnemyCount = 0;

    for (const Enemy& enemy : enemies)
    {
        if (!enemy.IsAlive())
        {
            continue;
        }

        Vector3 toCore =
            Vector3Subtract(
                position,
                enemy.GetPosition()
            );

        toCore.y = 0.0f;

        if (
            Vector3Length(toCore) <=
            enemyAttackRange
            )
        {
            attackingEnemyCount++;
        }
    }

    activeAttackerCount =
        std::min(
            attackingEnemyCount,
            maxSimultaneousAttackers
        );

    // Level basinda oyuncuya gorevi okuyup konum almasi icin zaman ver.
    if (protectionTimer > 0.0f)
    {
        damageTimer = 0.0f;
        activeAttackerCount = 0;
        return false;
    }

    if (activeAttackerCount == 0)
    {
        damageTimer = 0.0f;
        return false;
    }

    damageTimer -= deltaTime;

    if (damageTimer > 0.0f)
    {
        return false;
    }

    health -=
        damagePerEnemy *
        static_cast<float>(
            activeAttackerCount
        );

    if (health < 0.0f)
    {
        health = 0.0f;
    }

    damageTimer = damageInterval;
    hitFlashTimer = 0.22f;

    return true;
}

void DefenseCore::Draw() const
{
    const float pulse =
        0.5f +
        0.5f *
        sinf(visualTime * 4.0f);

    const bool flashing =
        hitFlashTimer > 0.0f;

    const Color energyColor =
        flashing
        ? Color{ 255, 92, 122, 255 }
        : Color{ 67, 225, 255, 255 };

    DrawCircle3D(
        Vector3{
            position.x,
            0.035f,
            position.z
        },
        3.0f,
        Vector3{ 1.0f, 0.0f, 0.0f },
        90.0f,
        Fade(energyColor, 0.17f)
    );

    DrawCircle3D(
        Vector3{
            position.x,
            0.045f,
            position.z
        },
        2.35f,
        Vector3{ 1.0f, 0.0f, 0.0f },
        90.0f,
        Fade(energyColor, 0.48f)
    );

    DrawCylinder(
        Vector3{
            position.x,
            0.28f,
            position.z
        },
        1.55f,
        1.25f,
        0.55f,
        12,
        Color{ 31, 37, 55, 255 }
    );

    DrawCylinderWires(
        Vector3{
            position.x,
            0.28f,
            position.z
        },
        1.55f,
        1.25f,
        0.55f,
        12,
        Color{ 100, 118, 153, 255 }
    );

    const float coreScale =
        0.78f +
        pulse * 0.10f;

    rlPushMatrix();
    rlTranslatef(
        position.x,
        position.y,
        position.z
    );
    rlRotatef(
        visualTime * 65.0f,
        0.0f,
        1.0f,
        0.0f
    );

    DrawCube(
        Vector3{ 0.0f, 0.0f, 0.0f },
        coreScale,
        coreScale,
        coreScale,
        energyColor
    );

    DrawCubeWires(
        Vector3{ 0.0f, 0.0f, 0.0f },
        coreScale + 0.16f,
        coreScale + 0.16f,
        coreScale + 0.16f,
        RAYWHITE
    );

    rlPopMatrix();

    for (int crystal = 0; crystal < 4; crystal++)
    {
        const float angle =
            visualTime * 1.2f +
            crystal * PI / 2.0f;

        DrawSphere(
            Vector3{
                position.x +
                    cosf(angle) * 1.15f,
                position.y +
                    sinf(
                        visualTime * 2.4f +
                        crystal
                    ) * 0.12f,
                position.z +
                    sinf(angle) * 1.15f
            },
            0.18f,
            Fade(energyColor, 0.88f)
        );
    }
}

void DefenseCore::DrawHUD() const
{
    float healthPercentage =
        health / maxHealth;

    if (healthPercentage < 0.0f)
    {
        healthPercentage = 0.0f;
    }

    if (healthPercentage > 1.0f)
    {
        healthPercentage = 1.0f;
    }

    const int panelWidth = 420;
    const int panelHeight = 64;
    const int panelX =
        GetScreenWidth() / 2 -
        panelWidth / 2;
    const int panelY = 82;

    DrawRectangleRounded(
        Rectangle{
            static_cast<float>(panelX),
            static_cast<float>(panelY),
            static_cast<float>(panelWidth),
            static_cast<float>(panelHeight)
        },
        0.16f,
        8,
        Color{ 5, 11, 23, 220 }
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
        Color{ 67, 225, 255, 255 }
    );

    const char* label = nullptr;

    if (protectionTimer > 0.0f)
    {
        label = TextFormat(
            "CORE SHIELD ACTIVE - GET READY  %.0f",
            ceilf(protectionTimer)
        );
    }
    else
    {
        label = TextFormat(
            "AETHER CORE  %d/%d   ATTACKERS %d/%d",
            static_cast<int>(health),
            static_cast<int>(maxHealth),
            activeAttackerCount,
            maxSimultaneousAttackers
        );
    }

    DrawText(
        label,
        panelX +
            panelWidth / 2 -
            MeasureText(label, 20) / 2,
        panelY + 8,
        20,
        RAYWHITE
    );

    DrawRectangle(
        panelX + 22,
        panelY + 38,
        panelWidth - 44,
        13,
        Color{ 30, 38, 54, 255 }
    );

    DrawRectangle(
        panelX + 22,
        panelY + 38,
        static_cast<int>(
            (panelWidth - 44) *
            healthPercentage
        ),
        13,
        healthPercentage > 0.35f
        ? Color{ 67, 225, 255, 255 }
        : Color{ 255, 82, 103, 255 }
    );

    if (activeAttackerCount > 0)
    {
        const char* warning =
            "CORE UNDER ATTACK! KILL THE ENEMY INSIDE THE CORE RING!";

        const int warningFontSize = 24;
        const int warningWidth =
            MeasureText(warning, warningFontSize);

        DrawRectangle(
            GetScreenWidth() / 2 - warningWidth / 2 - 18,
            panelY + panelHeight + 10,
            warningWidth + 36,
            42,
            Color{ 70, 5, 12, 220 }
        );

        DrawText(
            warning,
            GetScreenWidth() / 2 - warningWidth / 2,
            panelY + panelHeight + 18,
            warningFontSize,
            Color{ 255, 90, 105, 255 }
        );
    }
}

Vector3 DefenseCore::GetPosition() const
{
    return position;
}

float DefenseCore::GetHealth() const
{
    return health;
}

float DefenseCore::GetMaxHealth() const
{
    return maxHealth;
}

bool DefenseCore::IsAlive() const
{
    return health > 0.0f;
}
