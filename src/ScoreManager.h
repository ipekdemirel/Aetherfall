#pragma once

class ScoreManager
{
public:
    ScoreManager();

    void Reset();

    void AddScore(int amount);

    int GetScore() const;

private:
    int score;
};