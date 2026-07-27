#pragma once

#include <raylib.h>

#include <vector>

#include "Coin.h"

class CoinManager
{
public:
    CoinManager();

    void Reset();

    void SpawnCoin(const Vector3& position);

    int Update(
        float deltaTime,
        const Vector3& playerPosition
    );

    void Draw() const;

    int GetActiveCoinCount() const;

private:
    std::vector<Coin> coins;
};