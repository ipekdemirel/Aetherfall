#pragma once

#include <raylib.h>

class Player;

class Shop
{
public:
    Shop();

    void Update(
        Player& player,
        int& coinCount
    );

    void Draw(
        const Player& player,
        int coinCount
    ) const;

    bool IsOpen() const;

private:
    bool isOpen;

    float messageTimer;

    enum class MessageType
    {
        None,
        PurchaseSuccessful,
        NotEnoughCoins,
        HealthAlreadyFull
    };

    MessageType messageType;

    Rectangle GetHealButton() const;

    bool IsMouseOver(
        const Rectangle& rectangle
    ) const;
};