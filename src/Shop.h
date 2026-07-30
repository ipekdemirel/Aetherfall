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
    enum class UpgradeType
    {
        Heal,
        Damage,
        Armor,
        MaxHealth,
        Speed,
        Dash
    };

    enum class MessageType
    {
        None,
        Purchased,
        NotEnoughCoins,
        HealthAlreadyFull,
        MaximumLevel
    };

    Rectangle GetPanelRectangle() const;
    Rectangle GetUpgradeButton(int index) const;

    bool IsMouseOver(const Rectangle& rectangle) const;
    void ShowMessage(MessageType newMessage);
    void TryPurchase(
        UpgradeType type,
        Player& player,
        CombatSystem& combatSystem,
        int& coinCount
    );

    int GetLevel(UpgradeType type) const;
    int GetMaximumLevel(UpgradeType type) const;
    int GetCost(UpgradeType type) const;
    const char* GetName(UpgradeType type) const;

    bool isOpen;

    int damageLevel;
    int armorLevel;
    int maxHealthLevel;
    int speedLevel;
    int dashLevel;

    float messageTimer;
    MessageType messageType;
};
