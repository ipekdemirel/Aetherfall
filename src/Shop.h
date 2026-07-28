#pragma once

#include <raylib.h>

class CombatSystem;
class Player;

class Shop
{
public:
    Shop();

    void Update(
        Player& player,
        CombatSystem& combatSystem,
        int& coinCount
    );

    void Draw(
        const Player& player,
        const CombatSystem& combatSystem,
        int coinCount
    ) const;

    bool IsOpen() const;

private:
    enum class MessageType
    {
        None,
        HealthPurchased,
        DamagePurchased,
        NotEnoughCoins,
        HealthAlreadyFull
    };

    Rectangle GetPanelRectangle() const;
    Rectangle GetHealButton() const;
    Rectangle GetDamageButton() const;

    bool IsMouseOver(
        const Rectangle& rectangle
    ) const;

    void ShowMessage(
        MessageType newMessage
    );

    bool isOpen;

    float messageTimer;
    MessageType messageType;
};