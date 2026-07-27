#include "CoinManager.h"

CoinManager::CoinManager()
{
}

void CoinManager::Reset()
{
    coins.clear();
}

void CoinManager::SpawnCoin(
    const Vector3& position
)
{
    Vector3 coinPosition = position;

    coinPosition.y += 0.45f;

    coins.emplace_back(
        coinPosition
    );
}

int CoinManager::Update(
    float deltaTime,
    const Vector3& playerPosition
)
{
    int collectedCoinCount = 0;

    for (Coin& coin : coins)
    {
        const bool wasCollected =
            coin.IsCollected();

        coin.Update(
            deltaTime,
            playerPosition
        );

        if (
            !wasCollected &&
            coin.IsCollected()
            )
        {
            collectedCoinCount++;
        }
    }

    return collectedCoinCount;
}

void CoinManager::Draw() const
{
    for (const Coin& coin : coins)
    {
        coin.Draw();
    }
}

int CoinManager::GetActiveCoinCount() const
{
    int activeCoinCount = 0;

    for (const Coin& coin : coins)
    {
        if (!coin.IsCollected())
        {
            activeCoinCount++;
        }
    }

    return activeCoinCount;
}