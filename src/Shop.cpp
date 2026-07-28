#include "Shop.h"

#include "CombatSystem.h"
#include "Player.h"

Shop::Shop()
    :
    isOpen(false),
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
    const float panelWidth =
        760.0f;

    const float panelHeight =
        520.0f;

    return Rectangle{
        GetScreenWidth() / 2.0f -
        panelWidth / 2.0f,

        GetScreenHeight() / 2.0f -
        panelHeight / 2.0f,

        panelWidth,
        panelHeight
    };
}

Rectangle Shop::GetHealButton() const
{
    const Rectangle panel =
        GetPanelRectangle();

    return Rectangle{
        panel.x + 50.0f,
        panel.y + 150.0f,
        panel.width - 100.0f,
        80.0f
    };
}

Rectangle Shop::GetDamageButton() const
{
    const Rectangle panel =
        GetPanelRectangle();

    return Rectangle{
        panel.x + 50.0f,
        panel.y + 250.0f,
        panel.width - 100.0f,
        80.0f
    };
}

bool Shop::IsMouseOver(
    const Rectangle& rectangle
) const
{
    return CheckCollisionPointRec(
        GetMousePosition(),
        rectangle
    );
}

void Shop::ShowMessage(
    MessageType newMessage
)
{
    messageType =
        newMessage;

    messageTimer =
        2.0f;
}

void Shop::Update(
    Player& player,
    CombatSystem& combatSystem,
    int& coinCount
)
{
    const float deltaTime =
        GetFrameTime();

    if (messageTimer > 0.0f)
    {
        messageTimer -=
            deltaTime;

        if (messageTimer <= 0.0f)
        {
            messageTimer =
                0.0f;

            messageType =
                MessageType::None;
        }
    }

    if (IsKeyPressed(KEY_B))
    {
        isOpen =
            !isOpen;

        messageType =
            MessageType::None;

        messageTimer =
            0.0f;

        return;
    }

    if (!isOpen)
    {
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        isOpen =
            false;

        messageType =
            MessageType::None;

        messageTimer =
            0.0f;

        return;
    }

    if (
        IsMouseButtonPressed(
            MOUSE_BUTTON_LEFT
        )
        )
    {
        const Rectangle healButton =
            GetHealButton();

        const Rectangle damageButton =
            GetDamageButton();

        // =============================================
        // HEAL PURCHASE
        // =============================================

        if (IsMouseOver(healButton))
        {
            const int healCost =
                10;

            const int healAmount =
                25;

            if (
                player.GetHealth() >=
                player.GetMaxHealth()
                )
            {
                ShowMessage(
                    MessageType::HealthAlreadyFull
                );

                return;
            }

            if (coinCount < healCost)
            {
                ShowMessage(
                    MessageType::NotEnoughCoins
                );

                return;
            }

            coinCount -=
                healCost;

            player.Heal(
                healAmount
            );

            ShowMessage(
                MessageType::HealthPurchased
            );

            return;
        }

        // =============================================
        // DAMAGE PURCHASE
        // =============================================

        if (IsMouseOver(damageButton))
        {
            const int damageCost =
                25;

            const float damageIncrease =
                10.0f;

            if (coinCount < damageCost)
            {
                ShowMessage(
                    MessageType::NotEnoughCoins
                );

                return;
            }

            coinCount -=
                damageCost;

            combatSystem
                .IncreasePlayerAttackDamage(
                    damageIncrease
                );

            ShowMessage(
                MessageType::DamagePurchased
            );
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

    const int screenWidth =
        GetScreenWidth();

    const int screenHeight =
        GetScreenHeight();

    DrawRectangle(
        0,
        0,
        screenWidth,
        screenHeight,
        Color{
            0,
            0,
            0,
            140
        }
    );

    const Rectangle panel =
        GetPanelRectangle();

    DrawRectangleRec(
        panel,
        Color{
            15,
            18,
            22,
            248
        }
    );

    DrawRectangleLinesEx(
        panel,
        3.0f,
        GOLD
    );

    // =============================================
    // TITLE
    // =============================================

    const char* title =
        "SHOP";

    const int titleFontSize =
        44;

    const int titleWidth =
        MeasureText(
            title,
            titleFontSize
        );

    DrawText(
        title,
        static_cast<int>(
            panel.x +
            panel.width / 2.0f -
            titleWidth / 2.0f
            ),
        static_cast<int>(
            panel.y + 22.0f
            ),
        titleFontSize,
        GOLD
    );

    DrawText(
        TextFormat(
            "Coins: %d",
            coinCount
        ),
        static_cast<int>(
            panel.x + 50.0f
            ),
        static_cast<int>(
            panel.y + 95.0f
            ),
        25,
        GOLD
    );

    DrawText(
        TextFormat(
            "Health: %d / %d",
            player.GetHealth(),
            player.GetMaxHealth()
        ),
        static_cast<int>(
            panel.x +
            panel.width -
            280.0f
            ),
        static_cast<int>(
            panel.y + 95.0f
            ),
        25,
        GREEN
    );

    // =============================================
    // HEAL BUTTON
    // =============================================

    const Rectangle healButton =
        GetHealButton();

    const bool healButtonHovered =
        IsMouseOver(
            healButton
        );

    DrawRectangleRec(
        healButton,
        healButtonHovered
        ? Color{ 70, 70, 75, 255 }
        : Color{ 40, 40, 45, 255 }
    );

    DrawRectangleLinesEx(
        healButton,
        2.0f,
        healButtonHovered
        ? YELLOW
        : LIGHTGRAY
    );

    DrawText(
        "HEAL +25 HP",
        static_cast<int>(
            healButton.x + 25.0f
            ),
        static_cast<int>(
            healButton.y + 14.0f
            ),
        28,
        WHITE
    );

    DrawText(
        "10 COINS",
        static_cast<int>(
            healButton.x +
            healButton.width -
            165.0f
            ),
        static_cast<int>(
            healButton.y + 14.0f
            ),
        28,
        GOLD
    );

    DrawText(
        "Restore lost health",
        static_cast<int>(
            healButton.x + 25.0f
            ),
        static_cast<int>(
            healButton.y + 50.0f
            ),
        18,
        GRAY
    );

    // =============================================
    // DAMAGE BUTTON
    // =============================================

    const Rectangle damageButton =
        GetDamageButton();

    const bool damageButtonHovered =
        IsMouseOver(
            damageButton
        );

    DrawRectangleRec(
        damageButton,
        damageButtonHovered
        ? Color{ 70, 70, 75, 255 }
        : Color{ 40, 40, 45, 255 }
    );

    DrawRectangleLinesEx(
        damageButton,
        2.0f,
        damageButtonHovered
        ? YELLOW
        : LIGHTGRAY
    );

    DrawText(
        "SWORD DAMAGE +10",
        static_cast<int>(
            damageButton.x + 25.0f
            ),
        static_cast<int>(
            damageButton.y + 14.0f
            ),
        27,
        WHITE
    );

    DrawText(
        "25 COINS",
        static_cast<int>(
            damageButton.x +
            damageButton.width -
            165.0f
            ),
        static_cast<int>(
            damageButton.y + 14.0f
            ),
        28,
        GOLD
    );

    DrawText(
        TextFormat(
            "Current damage: %d",
            static_cast<int>(
                combatSystem
                .GetPlayerAttackDamage()
                )
        ),
        static_cast<int>(
            damageButton.x + 25.0f
            ),
        static_cast<int>(
            damageButton.y + 50.0f
            ),
        18,
        GRAY
    );

    // =============================================
    // MESSAGE
    // =============================================

    if (
        messageType ==
        MessageType::HealthPurchased
        )
    {
        DrawText(
            "Health restored!",
            static_cast<int>(
                panel.x + 50.0f
                ),
            static_cast<int>(
                panel.y + 355.0f
                ),
            25,
            GREEN
        );
    }
    else if (
        messageType ==
        MessageType::DamagePurchased
        )
    {
        DrawText(
            "Sword damage increased!",
            static_cast<int>(
                panel.x + 50.0f
                ),
            static_cast<int>(
                panel.y + 355.0f
                ),
            25,
            GREEN
        );
    }
    else if (
        messageType ==
        MessageType::NotEnoughCoins
        )
    {
        DrawText(
            "Not enough coins!",
            static_cast<int>(
                panel.x + 50.0f
                ),
            static_cast<int>(
                panel.y + 355.0f
                ),
            25,
            RED
        );
    }
    else if (
        messageType ==
        MessageType::HealthAlreadyFull
        )
    {
        DrawText(
            "Health is already full!",
            static_cast<int>(
                panel.x + 50.0f
                ),
            static_cast<int>(
                panel.y + 355.0f
                ),
            25,
            SKYBLUE
        );
    }

    DrawText(
        "B or ESC : Close Shop",
        static_cast<int>(
            panel.x + 50.0f
            ),
        static_cast<int>(
            panel.y +
            panel.height -
            48.0f
            ),
        22,
        LIGHTGRAY
    );
}