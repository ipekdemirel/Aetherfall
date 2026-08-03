#include "EnemyManager.h"

#include <cmath>

#include <raymath.h>

EnemyManager::EnemyManager()
    : currentWave(1),
    startingEnemyCount(5),
    extraEnemiesPerWave(2),
    nextWaveDelay(3.0f),
    nextWaveTimer(0.0f),
    waitingForNextWave(false),
    defenseMode(false),
    defenseComplete(false),
    totalWaves(0)
{
}

void EnemyManager::Initialize()
{
    currentWave = 1;

    nextWaveDelay = 3.0f;

    waitingForNextWave = false;

    nextWaveTimer = 0.0f;

    defenseMode = false;
    defenseComplete = false;
    totalWaves = 0;

    SpawnCurrentWave();
}

void EnemyManager::Reset()
{
    Initialize();
}

void EnemyManager::StartDefense(int waveCount)
{
    currentWave = 1;
    waitingForNextWave = false;
    nextWaveTimer = 0.0f;

    // Level 2 gives the player enough time to collect coins, reposition and
    // read the next-wave warning before another group arrives.
    nextWaveDelay = 6.0f;

    defenseMode = true;
    defenseComplete = false;
    totalWaves =
        waveCount > 0
        ? waveCount
        : 1;

    SpawnCurrentWave();
}

void EnemyManager::SpawnCurrentWave()
{
    enemies.clear();

    // Normal levels keep the original 5, 7, 9... progression. The defense
    // mission uses a fairer 4, 5, 6 progression because the player must also
    // protect the core while fighting.
    const int enemyCount =
        defenseMode
        ? 4 + (currentWave - 1)
        : startingEnemyCount +
            (currentWave - 1) *
            extraEnemiesPerWave;

    const float radius =
        12.0f;

    for (
        int i = 0;
        i < enemyCount;
        i++
        )
    {
        const float angle =
            DEG2RAD *
            (360.0f / enemyCount) *
            i;

        Vector3 position =
        {
            cosf(angle) * radius,
            1.0f,
            sinf(angle) * radius
        };

        enemies.emplace_back(
            position
        );
    }
}

bool EnemyManager::AreAllEnemiesDefeated() const
{
    for (
        const Enemy& enemy :
        enemies
        )
    {
        if (enemy.IsAlive())
        {
            return false;
        }
    }

    return true;
}

void EnemyManager::Update(
    float deltaTime,
    const Vector3& targetPosition,
    bool forceChaseTarget
)
{
    UpdateEnemies(
        deltaTime,
        targetPosition,
        forceChaseTarget,
        nullptr
    );

    UpdateWaveSystem(deltaTime);
}

void EnemyManager::UpdateDefense(
    float deltaTime,
    const Vector3& playerPosition,
    const Vector3& corePosition
)
{
    UpdateEnemies(
        deltaTime,
        corePosition,
        true,
        &playerPosition
    );

    UpdateWaveSystem(deltaTime);
}

void EnemyManager::UpdateEnemies(
    float deltaTime,
    const Vector3& targetPosition,
    bool forceChaseTarget,
    const Vector3* defensePlayerPosition
)
{
    // =====================================================
    // SEPARATION SETTINGS
    // =====================================================

    const float separationRadius =
        1.8f;

    const float separationStrength =
        1.65f;

    for (
        std::size_t i = 0;
        i < enemies.size();
        i++
        )
    {
        Enemy& currentEnemy =
            enemies[i];

        if (!currentEnemy.IsAlive())
        {
            currentEnemy.Update(
                deltaTime,
                targetPosition,
                Vector3{ 0.0f, 0.0f, 0.0f },
                forceChaseTarget
            );

            continue;
        }

        Vector3 separationForce =
        {
            0.0f,
            0.0f,
            0.0f
        };

        const Vector3 currentPosition =
            currentEnemy.GetPosition();

        Vector3 selectedTarget =
            targetPosition;

        bool selectedForceChase =
            forceChaseTarget;

        if (defensePlayerPosition != nullptr)
        {
            // Savunma bolumunde yalnizca her uc dusmandan biri cekirdege gider.
            // Diger dusmanlar oyuncuyla savasir. Boylece cekirdek arkada kendi
            // kendine erimez ve oyuncu normal savasina devam edebilir.
            const bool isCoreAttacker =
                i % 3 == 0;

            if (!isCoreAttacker)
            {
                selectedTarget =
                    *defensePlayerPosition;

                selectedForceChase = true;
            }

            // Cekirdek saldirganlarina cekirdegin cevresinde ayri yuvalar ver.
            // Boylece ayni koordinatin ustunde yigilmadan kusatirlar.
            const float slotAngle =
                static_cast<float>(i) * 2.39996323f;

            const float slotRadius =
                1.75f +
                static_cast<float>(i % 2) * 0.28f;

            if (isCoreAttacker)
            {
                selectedTarget =
                {
                    targetPosition.x +
                        cosf(slotAngle) * slotRadius,
                    targetPosition.y,
                    targetPosition.z +
                        sinf(slotAngle) * slotRadius
                };

                selectedForceChase = true;
            }
        }

        for (
            std::size_t j = 0;
            j < enemies.size();
            j++
            )
        {
            if (i == j)
            {
                continue;
            }

            const Enemy& otherEnemy =
                enemies[j];

            if (!otherEnemy.IsAlive())
            {
                continue;
            }

            Vector3 awayDirection =
                Vector3Subtract(
                    currentPosition,
                    otherEnemy.GetPosition()
                );

            awayDirection.y = 0.0f;

            const float distance =
                Vector3Length(
                    awayDirection
                );

            if (
                distance > 0.001f &&
                distance < separationRadius
                )
            {
                awayDirection =
                    Vector3Normalize(
                        awayDirection
                    );

                const float closeness =
                    1.0f -
                    distance /
                    separationRadius;

                separationForce =
                    Vector3Add(
                        separationForce,
                        Vector3Scale(
                            awayDirection,
                            closeness
                        )
                    );
            }
        }

        separationForce =
            Vector3Scale(
                separationForce,
                separationStrength
            );

        currentEnemy.Update(
            deltaTime,
            selectedTarget,
            separationForce,
            selectedForceChase
        );
    }
}

void EnemyManager::UpdateWaveSystem(float deltaTime)
{
    // =====================================================
    // WAVE SYSTEM
    // =====================================================

    if (!waitingForNextWave)
    {
        if (AreAllEnemiesDefeated())
        {
            if (
                defenseMode &&
                currentWave >= totalWaves
                )
            {
                defenseComplete = true;
                return;
            }

            waitingForNextWave = true;

            nextWaveTimer =
                nextWaveDelay;
        }
    }
    else
    {
        nextWaveTimer -=
            deltaTime;

        if (nextWaveTimer <= 0.0f)
        {
            currentWave++;

            waitingForNextWave =
                false;

            SpawnCurrentWave();
        }
    }
}

void EnemyManager::Draw() const
{
    for (
        const Enemy& enemy :
        enemies
        )
    {
        enemy.Draw();
    }
}

std::vector<Enemy>& EnemyManager::GetEnemies()
{
    return enemies;
}

const std::vector<Enemy>&
EnemyManager::GetEnemies() const
{
    return enemies;
}

int EnemyManager::GetCurrentWave() const
{
    return currentWave;
}

int EnemyManager::GetAliveEnemyCount() const
{
    int count = 0;

    for (
        const Enemy& enemy :
        enemies
        )
    {
        if (enemy.IsAlive())
        {
            count++;
        }
    }

    return count;
}

bool EnemyManager::IsWaitingForNextWave() const
{
    return waitingForNextWave;
}

float EnemyManager::GetNextWaveTimer() const
{
    return nextWaveTimer;
}

int EnemyManager::GetTotalWaves() const
{
    return totalWaves;
}

bool EnemyManager::IsDefenseComplete() const
{
    return defenseMode &&
        defenseComplete;
}
