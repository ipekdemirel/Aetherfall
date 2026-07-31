#include "EnemyManager.h"

#include <cmath>

#include <raymath.h>

EnemyManager::EnemyManager()
    : currentWave(1),
    startingEnemyCount(5),
    extraEnemiesPerWave(2),
    nextWaveDelay(3.0f),
    nextWaveTimer(0.0f),
    waitingForNextWave(false)
{
}

void EnemyManager::Initialize()
{
    currentWave = 1;

    waitingForNextWave = false;

    nextWaveTimer = 0.0f;

    SpawnCurrentWave();
}

void EnemyManager::Reset()
{
    Initialize();
}

void EnemyManager::SpawnCurrentWave()
{
    enemies.clear();

    const int enemyCount =
        startingEnemyCount +
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
    const Vector3& playerPosition
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
                playerPosition,
                Vector3{ 0.0f, 0.0f, 0.0f }
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
            playerPosition,
            separationForce
        );
    }

    // =====================================================
    // WAVE SYSTEM
    // =====================================================

    if (!waitingForNextWave)
    {
        if (AreAllEnemiesDefeated())
        {
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