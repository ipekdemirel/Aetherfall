#include "Shop.h"

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

Rectangle Shop::GetHealButton() const
{
    const float panelWidth = 760.0f;

    const float panelX =
        GetScreenWidth() / 2.0f -
        panelWidth / 2.0f;

    return Rectangle{
        panelX + 50.0f,
        225.0f,
        panelWidth - 100.0f,
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

void Shop::Update(
    Player& player,
    int& coinCount
)
{
    const float deltaTime =
        GetFrameTime();

    if (messageTimer > 0.0f)
    {
        messageTimer -= deltaTime;

        if (messageTimer <= 0.0f)
        {
            messageTimer = 0.0f;
            messageType = MessageType::None;
        }
    }

    if (IsKeyPressed(KEY_B))
    {
        isOpen = !isOpen;

        messageType =
            MessageType::None;

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

        messageType =
            MessageType::None;

        messageTimer = 0.0f;

        return;
    }

    const Rectangle healButton =
        GetHealButton();

    if (
        IsMouseOver(healButton) &&
        IsMouseButtonPressed(
            MOUSE_BUTTON_LEFT
        )
        )
    {
        const int healCost = 10;
        const int healAmount = 25;

        if (
            player.GetHealth() >=
            player.GetMaxHealth()
            )
        {
            messageType =
                MessageType::HealthAlreadyFull;

            messageTimer = 2.0f;

            return;
        }

        if (coinCount < healCost)
        {
            messageType =
                MessageType::NotEnoughCoins;

            messageTimer = 2.0f;

            return;
        }

        coinCount -= healCost;

        player.Heal(
            healAmount
        );

        messageType =
            MessageType::PurchaseSuccessful;

        messageTimer = 2.0f;
    }
}

void Shop::Draw(
    const Player& player,
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
            120
        }
    );

    const float panelWidth =
        760.0f;

    const float panelHeight =
        460.0f;

    const float panelX =
        screenWidth / 2.0f -
        panelWidth / 2.0f;

    const float panelY =
        screenHeight / 2.0f -
        panelHeight / 2.0f;

    DrawRectangle(
        static_cast<int>(panelX),
        static_cast<int>(panelY),
        static_cast<int>(panelWidth),
        static_cast<int>(panelHeight),
        Color{
            15,
            18,
            22,
            245
        }
    );

    DrawRectangleLinesEx(
        Rectangle{
            panelX,
            panelY,
            panelWidth,
            panelHeight
        },
        3.0f,
        GOLD
    );

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
            panelX +
            panelWidth / 2.0f -
            titleWidth / 2.0f
            ),
        static_cast<int>(
            panelY + 25.0f
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
            panelX + 50.0f
            ),
        static_cast<int>(
            panelY + 95.0f
            ),
        26,
        GOLD
    );

    DrawText(
        TextFormat(
            "Health: %d / %d",
            player.GetHealth(),
            player.GetMaxHealth()
        ),
        static_cast<int>(
            panelX + panelWidth - 260.0f
            ),
        static_cast<int>(
            panelY + 95.0f
            ),
        26,
        GREEN
    );

    const Rectangle healButton =
        GetHealButton();

    const bool mouseOver =
        IsMouseOver(
            healButton
        );

    const Color buttonColor =
        mouseOver
        ? Color{
            70,
            70,
            75,
            255
    }
        : Color{
            40,
            40,
            45,
            255
    };

    DrawRectangleRec(
        healButton,
        buttonColor
    );

    DrawRectangleLinesEx(
        healButton,
        2.0f,
        mouseOver
        ? YELLOW
        : LIGHTGRAY
    );

    DrawText(
        "HEAL +25 HP",
        static_cast<int>(
            healButton.x + 25.0f
            ),
        static_cast<int>(
            healButton.y + 16.0f
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
            healButton.y + 16.0f
            ),
        28,
        GOLD
    );

    DrawText(
        "Click to purchase",
        static_cast<int>(
            healButton.x + 25.0f
            ),
        static_cast<int>(
            healButton.y + 50.0f
            ),
        18,
        GRAY
    );

    if (
        messageType ==
        MessageType::PurchaseSuccessful
        )
    {
        DrawText(
            "Health restored!",
            static_cast<int>(
                panelX + 50.0f
                ),
            static_cast<int>(
                panelY + 330.0f
                ),
            26,
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
                panelX + 50.0f
                ),
            static_cast<int>(
                panelY + 330.0f
                ),
            26,
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
                panelX + 50.0f
                ),
            static_cast<int>(
                panelY + 330.0f
                ),
            26,
            SKYBLUE
        );
    }

    DrawText(
        "B or ESC : Close Shop",
        static_cast<int>(
            panelX + 50.0f
            ),
        static_cast<int>(
            panelY +
            panelHeight -
            50.0f
            ),
        22,
        LIGHTGRAY
    );
}