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
        const Vector3& targetPosition,
        bool forceChaseTarget = false
    );

    void UpdateDefense(
        float deltaTime,
        const Vector3& playerPosition,
        const Vector3& corePosition
    );

    void StartDefense(int waveCount);

    void Draw() const;

    std::vector<Enemy>& GetEnemies();

    const std::vector<Enemy>& GetEnemies() const;

    int GetCurrentWave() const;
    int GetAliveEnemyCount() const;

    bool IsWaitingForNextWave() const;
    float GetNextWaveTimer() const;

    int GetTotalWaves() const;
    bool IsDefenseComplete() const;

private:
    void SpawnCurrentWave();

    void UpdateEnemies(
        float deltaTime,
        const Vector3& targetPosition,
        bool forceChaseTarget,
        const Vector3* defensePlayerPosition
    );

    void UpdateWaveSystem(float deltaTime);

    bool AreAllEnemiesDefeated() const;

    std::vector<Enemy> enemies;

    int currentWave;

    int startingEnemyCount;
    int extraEnemiesPerWave;

    float nextWaveDelay;
    float nextWaveTimer;

    bool waitingForNextWave;

    bool defenseMode;
    bool defenseComplete;
    int totalWaves;
};
