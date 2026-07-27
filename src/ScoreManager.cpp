#include "ScoreManager.h"

ScoreManager::ScoreManager()
    : score(0)
{
}

void ScoreManager::Reset()
{
    score = 0;
}

void ScoreManager::AddScore(int amount)
{
    score += amount;
}

int ScoreManager::GetScore() const
{
    return score;
}