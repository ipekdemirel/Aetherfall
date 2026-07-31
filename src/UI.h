#pragma once

#include <raylib.h>

class Enemy;
class EnemyManager;
class Player;

namespace UI
{
    void DrawHUD(
        const Player& player,
        const char* objectiveName,
        int objectiveProgress,
        int objectiveTotal
    );

    void DrawAltarInteraction(
        float activationProgress
    );

    void DrawEnemyHealthBar(
        const Enemy& enemy,
        const Camera3D& camera
    );

    void DrawWaveInformation(
        const EnemyManager& enemyManager
    );

    // ==========================
    // SCORE
    // ==========================

    void DrawScore(
        int score
    );

    // ==========================
    // COINS
    // ==========================

    void DrawCoins(
        int coins
    );
}