#include "Shop.h"

#include "CombatSystem.h"
#include "Player.h"

#include <cstdio>

namespace
{
    constexpr int HealCost = 10;
    constexpr int HealAmount = 25;

    constexpr int UpgradeCount = 6;

    const Color PanelColor{ 18, 22, 35, 248 };
    const Color CardColor{ 31, 38, 58, 255 };
    const Color CardHoverColor{ 42, 52, 78, 255 };
    const Color GoldColor{ 242, 190, 75, 255 };
    const Color AccentColor{ 91, 192, 235, 255 };
    const Color SuccessColor{ 90, 210, 130, 255 };
    const Color ErrorColor{ 245, 100, 100, 255 };
}

Shop::Shop()
    :
    isOpen(false),
    damageLevel(0),
    armorLevel(0),
    maxHealthLevel(0),
    speedLevel(0),
    dashLevel(0),
    messageTimer(0.0f),
    messageType(MessageType::None)
{
}

bool Shop::IsOpen() const
{
    return isOpen;
}

Rectangle Shop::GetPanelRectangle() const
{
    const float width = 900.0f;
    const float height = 650.0f;

    return Rectangle{
        GetScreenWidth() / 2.0f - width / 2.0f,
        GetScreenHeight() / 2.0f - height / 2.0f,
        width,
        height
    };
}

Rectangle Shop::GetUpgradeButton(int index) const
{
    const Rectangle panel = GetPanelRectangle();
    const int column = index % 2;
    const int row = index / 2;

    return Rectangle{
        panel.x + 40.0f + column * 420.0f,
        panel.y + 130.0f + row * 118.0f,
        400.0f,
        96.0f
    };
}

bool Shop::IsMouseOver(const Rectangle& rectangle) const
{
    return CheckCollisionPointRec(
        GetMousePosition(),
        rectangle
    );
}

void Shop::ShowMessage(MessageType newMessage)
{
    messageType = newMessage;
    messageTimer = 2.0f;
}

int Shop::GetLevel(UpgradeType type) const
{
    switch (type)
    {
    case UpgradeType::Damage:
        return damageLevel;
    case UpgradeType::Armor:
        return armorLevel;
    case UpgradeType::MaxHealth:
        return maxHealthLevel;
    case UpgradeType::Speed:
        return speedLevel;
    case UpgradeType::Dash:
        return dashLevel;
    default:
        return 0;
    }
}

int Shop::GetMaximumLevel(UpgradeType type) const
{
    if (type == UpgradeType::Heal)
    {
        return 0;
    }

    return 5;
}

int Shop::GetCost(UpgradeType type) const
{
    const int level = GetLevel(type);

    switch (type)
    {
    case UpgradeType::Heal:
        return HealCost;
    case UpgradeType::Damage:
        return 25 + level * 15;
    case UpgradeType::Armor:
        return 20 + level * 15;
    case UpgradeType::MaxHealth:
        return 30 + level * 20;
    case UpgradeType::Speed:
        return 25 + level * 20;
    case UpgradeType::Dash:
        return 35 + level * 25;
    }

    return 0;
}

const char* Shop::GetName(UpgradeType type) const
{
    switch (type)
    {
    case UpgradeType::Heal:
        return "HEAL";
    case UpgradeType::Damage:
        return "DAMAGE";
    case UpgradeType::Armor:
        return "ARMOR";
    case UpgradeType::MaxHealth:
        return "MAX HP";
    case UpgradeType::Speed:
        return "SPEED";
    case UpgradeType::Dash:
        return "DASH";
    }

    return "";
}

void Shop::TryPurchase(
    UpgradeType type,
    Player& player,
    CombatSystem& combatSystem,
    int& coinCount
)
{
    if (
        type == UpgradeType::Heal &&
        player.GetHealth() >= player.GetMaxHealth()
        )
    {
        ShowMessage(MessageType::HealthAlreadyFull);
        return;
    }

    const int maximumLevel = GetMaximumLevel(type);

    if (
        maximumLevel > 0 &&
        GetLevel(type) >= maximumLevel
        )
    {
        ShowMessage(MessageType::MaximumLevel);
        return;
    }

    const int cost = GetCost(type);

    if (coinCount < cost)
    {
        ShowMessage(MessageType::NotEnoughCoins);
        return;
    }

    coinCount -= cost;

    switch (type)
    {
    case UpgradeType::Heal:
        player.Heal(HealAmount);
        break;

    case UpgradeType::Damage:
        combatSystem.IncreasePlayerAttackDamage(5.0f);
        ++damageLevel;
        break;

    case UpgradeType::Armor:
        player.IncreaseArmor(0.05f);
        ++armorLevel;
        break;

    case UpgradeType::MaxHealth:
        player.IncreaseMaxHealth(20);
        ++maxHealthLevel;
        break;

    case UpgradeType::Speed:
        player.IncreaseMovementSpeed(0.4f, 0.5f);
        ++speedLevel;
        break;

    case UpgradeType::Dash:
        player.ReduceDashCooldown(0.08f);
        ++dashLevel;
        break;
    }

    ShowMessage(MessageType::Purchased);
}

void Shop::Update(
    Player& player,
    CombatSystem& combatSystem,
    int& coinCount
)
{
    if (messageTimer > 0.0f)
    {
        messageTimer -= GetFrameTime();

        if (messageTimer <= 0.0f)
        {
            messageTimer = 0.0f;
            messageType = MessageType::None;
        }
    }

    if (IsKeyPressed(KEY_B))
    {
        isOpen = !isOpen;
        messageType = MessageType::None;
        messageTimer = 0.0f;
        return;
    }

    if (!isOpen)
    {
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        isOpen = false;
        messageType = MessageType::None;
        messageTimer = 0.0f;
        return;
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    for (int index = 0; index < UpgradeCount; ++index)
    {
        if (IsMouseOver(GetUpgradeButton(index)))
        {
            TryPurchase(
                static_cast<UpgradeType>(index),
                player,
                combatSystem,
                coinCount
            );

            return;
        }
    }
}

void Shop::Draw(
    const Player& player,
    const CombatSystem& combatSystem,
    int coinCount
) const
{
    if (!isOpen)
    {
        return;
    }

    DrawRectangle(
        0,
        0,
        GetScreenWidth(),
        GetScreenHeight(),
        Color{ 0, 0, 0, 170 }
    );

    const Rectangle panel = GetPanelRectangle();

    DrawRectangleRounded(
        panel,
        0.035f,
        12,
        PanelColor
    );

    DrawRectangleRoundedLinesEx(
        panel,
        0.035f,
        12,
        3.0f,
        GoldColor
    );

    DrawText(
        "AETHER SHOP",
        static_cast<int>(panel.x + 40.0f),
        static_cast<int>(panel.y + 28.0f),
        36,
        GoldColor
    );

    char coinText[64];
    std::snprintf(
        coinText,
        sizeof(coinText),
        "COINS: %d",
        coinCount
    );

    const int coinTextWidth = MeasureText(coinText, 26);

    DrawText(
        coinText,
        static_cast<int>(panel.x + panel.width - 40.0f - coinTextWidth),
        static_cast<int>(panel.y + 35.0f),
        26,
        GoldColor
    );

    DrawText(
        "Buy upgrades to strengthen your character",
        static_cast<int>(panel.x + 42.0f),
        static_cast<int>(panel.y + 78.0f),
        19,
        LIGHTGRAY
    );

    for (int index = 0; index < UpgradeCount; ++index)
    {
        const UpgradeType type =
            static_cast<UpgradeType>(index);

        const Rectangle button =
            GetUpgradeButton(index);

        const bool hovered =
            IsMouseOver(button);

        const int level = GetLevel(type);
        const int maximumLevel = GetMaximumLevel(type);
        const bool isMaximum =
            maximumLevel > 0 &&
            level >= maximumLevel;

        DrawRectangleRounded(
            button,
            0.12f,
            8,
            hovered ? CardHoverColor : CardColor
        );

        DrawRectangleRoundedLinesEx(
            button,
            0.12f,
            8,
            hovered ? 3.0f : 1.5f,
            hovered ? AccentColor : Color{ 72, 88, 120, 255 }
        );

        DrawText(
            GetName(type),
            static_cast<int>(button.x + 18.0f),
            static_cast<int>(button.y + 15.0f),
            24,
            WHITE
        );

        char levelText[32];

        if (type == UpgradeType::Heal)
        {
            std::snprintf(
                levelText,
                sizeof(levelText),
                "+%d HP",
                HealAmount
            );
        }
        else
        {
            std::snprintf(
                levelText,
                sizeof(levelText),
                "LV %d / %d",
                level,
                maximumLevel
            );
        }

        DrawText(
            levelText,
            static_cast<int>(button.x + 18.0f),
            static_cast<int>(button.y + 56.0f),
            18,
            LIGHTGRAY
        );

        char costText[32];

        if (isMaximum)
        {
            std::snprintf(
                costText,
                sizeof(costText),
                "MAX"
            );
        }
        else
        {
            std::snprintf(
                costText,
                sizeof(costText),
                "%d COIN",
                GetCost(type)
            );
        }

        const int costWidth =
            MeasureText(costText, 20);

        DrawText(
            costText,
            static_cast<int>(
                button.x +
                button.width -
                18.0f -
                costWidth
                ),
            static_cast<int>(button.y + 60.0f),
            20,
            isMaximum ? SuccessColor : GoldColor
        );
    }

    const float statsY = panel.y + 505.0f;

    DrawLineEx(
        Vector2{ panel.x + 40.0f, statsY - 18.0f },
        Vector2{ panel.x + panel.width - 40.0f, statsY - 18.0f },
        2.0f,
        Color{ 60, 72, 100, 255 }
    );

    char statsLineOne[160];
    std::snprintf(
        statsLineOne,
        sizeof(statsLineOne),
        "HP: %d / %d     DAMAGE: %.0f     ARMOR: %.0f%%",
        player.GetHealth(),
        player.GetMaxHealth(),
        combatSystem.GetPlayerAttackDamage(),
        player.GetArmor() * 100.0f
    );

    DrawText(
        statsLineOne,
        static_cast<int>(panel.x + 40.0f),
        static_cast<int>(statsY),
        21,
        WHITE
    );

    char statsLineTwo[160];
    std::snprintf(
        statsLineTwo,
        sizeof(statsLineTwo),
        "WALK: %.1f     RUN: %.1f     DASH COOLDOWN: %.2f s",
        player.GetWalkSpeed(),
        player.GetRunSpeed(),
        player.GetDashCooldown()
    );

    DrawText(
        statsLineTwo,
        static_cast<int>(panel.x + 40.0f),
        static_cast<int>(statsY + 36.0f),
        21,
        WHITE
    );

    const char* message = "";
    Color messageColor = WHITE;

    switch (messageType)
    {
    case MessageType::Purchased:
        message = "Purchase successful!";
        messageColor = SuccessColor;
        break;
    case MessageType::NotEnoughCoins:
        message = "Not enough coins!";
        messageColor = ErrorColor;
        break;
    case MessageType::HealthAlreadyFull:
        message = "Health is already full!";
        messageColor = ErrorColor;
        break;
    case MessageType::MaximumLevel:
        message = "This upgrade is already at maximum level!";
        messageColor = GoldColor;
        break;
    default:
        break;
    }

    if (messageType != MessageType::None)
    {
        const int messageWidth =
            MeasureText(message, 20);

        DrawText(
            message,
            static_cast<int>(
                panel.x +
                panel.width / 2.0f -
                messageWidth / 2.0f
                ),
            static_cast<int>(panel.y + 592.0f),
            20,
            messageColor
        );
    }

    DrawText(
        "B or ESC: Close",
        static_cast<int>(panel.x + 40.0f),
        static_cast<int>(panel.y + panel.height - 30.0f),
        17,
        GRAY
    );
}
