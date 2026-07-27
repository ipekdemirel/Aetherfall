#pragma once

#include <vector>

#include <raylib.h>

#include "Enemy.h"

class EnemyManager
{
public:
    EnemyManager();

    void Initialize();
    void Reset();

    void Update(
        float deltaTime,
        const Vector3& playerPosition
    );

    void Draw() const;

    std::vector<Enemy>& GetEnemies();

    const std::vector<Enemy>& GetEnemies() const;

    int GetCurrentWave() const;
    int GetAliveEnemyCount() const;

    bool IsWaitingForNextWave() const;
    float GetNextWaveTimer() const;

private:
    void SpawnCurrentWave();

    bool AreAllEnemiesDefeated() const;

    std::vector<Enemy> enemies;

    int currentWave;

    int startingEnemyCount;
    int extraEnemiesPerWave;

    float nextWaveDelay;
    float nextWaveTimer;

    bool waitingForNextWave;
};