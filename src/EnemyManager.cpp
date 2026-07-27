#include "EnemyManager.h"

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
        (currentWave - 1) * extraEnemiesPerWave;

    const float radius =
        12.0f;

    for (int i = 0; i < enemyCount; i++)
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

        enemies.emplace_back(position);
    }
}

bool EnemyManager::AreAllEnemiesDefeated() const
{
    for (const Enemy& enemy : enemies)
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
    for (Enemy& enemy : enemies)
    {
        enemy.Update(
            deltaTime,
            playerPosition
        );
    }

    if (!waitingForNextWave)
    {
        if (AreAllEnemiesDefeated())
        {
            waitingForNextWave = true;
            nextWaveTimer = nextWaveDelay;
        }
    }
    else
    {
        nextWaveTimer -= deltaTime;

        if (nextWaveTimer <= 0.0f)
        {
            currentWave++;
            waitingForNextWave = false;
            SpawnCurrentWave();
        }
    }
}

void EnemyManager::Draw() const
{
    for (const Enemy& enemy : enemies)
    {
        enemy.Draw();
    }
}

std::vector<Enemy>& EnemyManager::GetEnemies()
{
    return enemies;
}

const std::vector<Enemy>& EnemyManager::GetEnemies() const
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

    for (const Enemy& enemy : enemies)
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