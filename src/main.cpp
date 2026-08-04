#include <raylib.h>
#include <raymath.h>

#include <cmath>
#include <vector>

#include "AetherAltar.h"
#include "AetherCore.h"
#include "AetherShatteredGate_LEVEL2_V8.h"
#include "Boss.h"
#include "BossHUD.h"
#include "CombatSystem.h"
#include "CoinManager.h"
#include "DefenseCore.h"
#include "EnemyManager.h"
#include "ExitDoor.h"
#include "GameState.h"
#include "Player.h"
#include "ScoreManager.h"
#include "Shop.h"
#include "UI.h"
#include "WorldEnvironment.h"

enum class ApplicationScreen
{
    MainMenu,
    StoryIntro,
    Controls,
    Gameplay
};

static bool DrawMenuButton(
    const char* text,
    Rectangle bounds,
    bool selected
)
{
    const Vector2 mousePosition =
        GetMousePosition();

    const bool hovered =
        CheckCollisionPointRec(
            mousePosition,
            bounds
        );

    const Color backgroundColor =
        hovered || selected
        ? Color{ 42, 126, 145, 235 }
    : Color{ 12, 28, 42, 220 };

    const Color borderColor =
        hovered || selected
        ? Color{ 108, 232, 255, 255 }
    : Color{ 54, 93, 112, 255 };

    DrawRectangleRounded(
        bounds,
        0.18f,
        8,
        backgroundColor
    );

    DrawRectangleRoundedLinesEx(
        bounds,
        0.18f,
        8,
        2.0f,
        borderColor
    );

    const int fontSize = 24;
    const int textWidth =
        MeasureText(text, fontSize);

    DrawText(
        text,
        static_cast<int>(
            bounds.x +
            bounds.width / 2.0f -
            textWidth / 2.0f
            ),
        static_cast<int>(
            bounds.y +
            bounds.height / 2.0f -
            fontSize / 2.0f
            ),
        fontSize,
        RAYWHITE
    );

    return
        hovered &&
        IsMouseButtonPressed(
            MOUSE_BUTTON_LEFT
        );
}

static void DrawAtmosphericBackground(
    float animationTime
)
{
    const int width =
        GetScreenWidth();

    const int height =
        GetScreenHeight();

    DrawRectangleGradientV(
        0,
        0,
        width,
        height,
        Color{ 5, 10, 22, 255 },
        Color{ 13, 42, 55, 255 }
    );

    for (int i = 0; i < 34; i++)
    {
        const float speed =
            7.0f + (i % 6) * 2.0f;

        const float x =
            static_cast<float>(
                (i * 97) %
                (width + 80)
                ) - 40.0f;

        const float y =
            fmodf(
                static_cast<float>(
                    i * 71
                    ) -
                animationTime * speed,
                static_cast<float>(
                    height + 100
                    )
            );

        const float wrappedY =
            y < -50.0f
            ? y + height + 100.0f
            : y;

        const float pulse =
            0.55f +
            sinf(
                animationTime * 2.0f +
                i
            ) * 0.25f;

        DrawCircleV(
            Vector2{ x, wrappedY },
            1.5f + (i % 3),
            Fade(
                SKYBLUE,
                pulse
            )
        );
    }

    const Vector2 coreCenter{
        width * 0.76f,
        height * 0.42f
    };

    const float corePulse =
        34.0f +
        sinf(animationTime * 2.4f) *
        5.0f;

    DrawCircleGradient(
        static_cast<int>(coreCenter.x),
        static_cast<int>(coreCenter.y),
        corePulse * 3.5f,
        Fade(SKYBLUE, 0.22f),
        BLANK
    );

    DrawPoly(
        coreCenter,
        6,
        corePulse,
        animationTime * 32.0f,
        Color{ 84, 226, 255, 255 }
    );

    DrawPolyLinesEx(
        coreCenter,
        6,
        corePulse + 11.0f,
        -animationTime * 24.0f,
        3.0f,
        RAYWHITE
    );

    DrawTriangle(
        Vector2{
            width * 0.47f,
            height * 0.78f
        },
        Vector2{
            width * 0.72f,
            height * 0.44f
        },
        Vector2{
            width * 0.90f,
            height * 0.78f
        },
        Color{ 8, 19, 29, 235 }
    );

    DrawTriangle(
        Vector2{
            width * 0.62f,
            height * 0.78f
        },
        Vector2{
            width * 0.86f,
            height * 0.53f
        },
        Vector2{
            width * 1.04f,
            height * 0.78f
        },
        Color{ 10, 30, 38, 245 }
    );

    DrawRectangleGradientV(
        0,
        static_cast<int>(
            height * 0.72f
            ),
        width,
        static_cast<int>(
            height * 0.28f
            ),
        Fade(
            Color{ 15, 59, 69, 255 },
            0.40f
        ),
        Color{ 3, 8, 15, 255 }
    );
}

static void DrawStoryPage(
    int storyPage,
    float storyPageTime
)
{
    const int width =
        GetScreenWidth();

    const int height =
        GetScreenHeight();

    DrawRectangleGradientV(
        0,
        0,
        width,
        height,
        Color{ 3, 6, 15, 255 },
        Color{ 11, 31, 43, 255 }
    );

    // Uzak dağlar ve sis, hikâye ekranına sinematik bir sahne verir.
    DrawTriangle(
        Vector2{ 0.0f, height * 0.72f },
        Vector2{ width * 0.27f, height * 0.28f },
        Vector2{ width * 0.55f, height * 0.72f },
        Color{ 8, 18, 30, 255 }
    );

    DrawTriangle(
        Vector2{ width * 0.32f, height * 0.72f },
        Vector2{ width * 0.67f, height * 0.36f },
        Vector2{
    static_cast<float>(width),
    static_cast<float>(height) * 0.72f
        },
        Color{ 10, 25, 36, 255 }
    );

    DrawRectangleGradientV(
        0,
        static_cast<int>(height * 0.62f),
        width,
        static_cast<int>(height * 0.38f),
        Fade(Color{ 32, 94, 105, 255 }, 0.32f),
        Color{ 2, 5, 12, 255 }
    );

    DrawCircleGradient(
        width / 2,
        height / 2 - 90,
        210.0f,
        Fade(SKYBLUE, 0.18f),
        BLANK
    );

    const float corePulse =
        24.0f + sinf(storyPageTime * 2.2f) * 4.0f;

    DrawPoly(
        Vector2{
            width / 2.0f,
            height / 2.0f - 90.0f
        },
        6,
        corePulse,
        storyPageTime * 20.0f,
        Fade(SKYBLUE, 0.55f)
    );

    const char* chapterText =
        storyPage == 0
        ? "THE FALL"
        : storyPage == 1
        ? "THE LAST WARDEN"
        : "YOUR JOURNEY";

    const char* lineOne =
        storyPage == 0
        ? "Once, Aether held the shattered realms together."
        : storyPage == 1
        ? "You awaken as the last Warden of a dying world."
        : "Find the lost shards. Break the corruption.";

    const char* lineTwo =
        storyPage == 0
        ? "Then its heart broke, and corruption entered the world."
        : storyPage == 1
        ? "A forgotten blade answers your call."
        : "Defeat the Aether Titan and reclaim the Core.";

    const char* lineThree =
        storyPage == 0
        ? "Ruins fell silent. Forests twisted. Guardians turned hostile."
        : storyPage == 1
        ? "Only you can gather the lost Aether shards."
        : "The fate of Aetherfall begins with your first step.";

    const int chapterSize = 42;
    const int chapterWidth =
        MeasureText(
            chapterText,
            chapterSize
        );

    const float titleFade =
        Clamp(storyPageTime / 0.8f, 0.0f, 1.0f);

    DrawText(
        chapterText,
        width / 2 -
        chapterWidth / 2,
        height / 2 - 185,
        chapterSize,
        Fade(
            Color{ 104, 226, 255, 255 },
            titleFade
        )
    );

    const char* storyLines[] = {
        lineOne,
        lineTwo,
        lineThree
    };

    for (int i = 0; i < 3; i++)
    {
        const int fontSize = 23;
        const int lineWidth =
            MeasureText(
                storyLines[i],
                fontSize
            );

        const float lineFade =
            Clamp(
                (storyPageTime - 0.65f - i * 0.55f) /
                0.75f,
                0.0f,
                1.0f
            );

        DrawText(
            storyLines[i],
            width / 2 -
            lineWidth / 2,
            height / 2 -
            60 +
            i * 44,
            fontSize,
            Fade(
                i == 2
                ? Color{ 151, 190, 202, 255 }
                : RAYWHITE,
                lineFade
            )
        );
    }

    const char* continueText =
        storyPage < 2
        ? "ENTER  -  CONTINUE"
        : "ENTER  -  BEGIN CHAPTER I";

    const int continueSize = 20;
    const int continueWidth =
        MeasureText(
            continueText,
            continueSize
        );

    DrawText(
        continueText,
        width / 2 -
        continueWidth / 2,
        height - 90,
        continueSize,
        Fade(
            Color{ 108, 232, 255, 255 },
            Clamp(
                (storyPageTime - 2.2f) /
                0.8f,
                0.0f,
                1.0f
            )
        )
    );

    DrawText(
        "ESC  -  SKIP",
        32,
        height - 48,
        17,
        Fade(
            GRAY,
            Clamp(
                (storyPageTime - 2.2f) /
                0.8f,
                0.0f,
                1.0f
            )
        )
    );

    // Üst ve alt sinema şeritleri.
    DrawRectangle(0, 0, width, 42, BLACK);
    DrawRectangle(0, height - 42, width, 42, BLACK);

    // Her yeni sayfanın yumuşak biçimde karanlıktan açılması.
    const float openingFade =
        1.0f -
        Clamp(storyPageTime / 0.9f, 0.0f, 1.0f);

    DrawRectangle(
        0,
        0,
        width,
        height,
        Fade(BLACK, openingFade)
    );
}

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(
        FLAG_MSAA_4X_HINT |
        FLAG_WINDOW_RESIZABLE
    );

    InitWindow(
        screenWidth,
        screenHeight,
        "Aetherfall"
    );

    SetExitKey(KEY_NULL);

    SetTargetFPS(60);

    Player player;

    std::vector<AetherAltar> altars;

    // =====================================================
    // LEVEL 1 AETHER ALTARS
    // =====================================================

    altars.emplace_back(
        Vector3{ -10.0f, 0.5f, -8.0f }
    );

    altars.emplace_back(
        Vector3{ 10.0f, 0.5f, -6.0f }
    );

    altars.emplace_back(
        Vector3{ 0.0f, 0.5f, 10.0f }
    );

    const int totalAltars = 3;
    const int totalDefenseWaves = 3;

    // =====================================================
    // LEVEL 1 EXIT DOOR
    // =====================================================

    ExitDoor exitDoor(
        Vector3{ 18.0f, 1.5f, 0.0f }
    );

    EnemyManager enemyManager;
    enemyManager.Initialize();

    CombatSystem combatSystem;

    CoinManager coinManager;

    ScoreManager scoreManager;

    Shop shop;

    WorldEnvironment worldEnvironment;

    // Boss gövde merkezi yerden yüksekte olmalıdır.
    Boss boss(
        Vector3{ 0.0f, 3.3f, -10.0f }
    );

    bool bossFightActive = false;
    bool bossRewardGranted = false;

    AetherCore aetherCore;
    DefenseCore defenseCore;
    AetherSealbreaker aetherSealbreaker;

    GameState gameState =
        GameState::Playing;

    int currentLevel = 1;

    const Vector3 cameraOffset{
        0.0f,
        10.0f,
        12.0f
    };

    Camera3D camera{};

    camera.position = {
        player.GetPosition().x + cameraOffset.x,
        player.GetPosition().y + cameraOffset.y,
        player.GetPosition().z + cameraOffset.z
    };

    camera.target =
        player.GetPosition();

    camera.up = {
        0.0f,
        1.0f,
        0.0f
    };

    camera.fovy = 45.0f;

    camera.projection =
        CAMERA_PERSPECTIVE;

    const float cameraFollowSpeed = 7.0f;

    float cameraShakeTime = 0.0f;

    const float cameraShakeDuration = 0.10f;
    const float cameraShakeStrength = 0.18f;

    float lavaDamageTimer = 0.0f;

    int lastHealedDefenseWave = 0;
    float levelCompleteAutoTimer = 0.0f;

    int activatedAltars = 0;
    int coinCount = 0;

    ApplicationScreen applicationScreen =
        ApplicationScreen::MainMenu;

    int selectedMenuItem = 0;
    int storyPage = 0;
    float storyPageTime = 0.0f;
    float menuAnimationTime = 0.0f;
    bool requestExit = false;

    while (
        !WindowShouldClose() &&
        !requestExit
        )
    {
        const float deltaTime =
            GetFrameTime();

        menuAnimationTime += deltaTime;
        worldEnvironment.Update(deltaTime);

        if (
            applicationScreen ==
            ApplicationScreen::MainMenu
            )
        {
            if (
                IsKeyPressed(KEY_DOWN) ||
                IsKeyPressed(KEY_S)
                )
            {
                selectedMenuItem =
                    (selectedMenuItem + 1) % 3;
            }

            if (
                IsKeyPressed(KEY_UP) ||
                IsKeyPressed(KEY_W)
                )
            {
                selectedMenuItem =
                    (selectedMenuItem + 2) % 3;
            }

            BeginDrawing();

            DrawAtmosphericBackground(
                menuAnimationTime
            );

            const int width =
                GetScreenWidth();

            const int height =
                GetScreenHeight();

            DrawText(
                "AETHERFALL",
                80,
                static_cast<int>(
                    height * 0.18f
                    ),
                72,
                RAYWHITE
            );

            DrawText(
                "ECHOES OF THE SHATTERED CORE",
                84,
                static_cast<int>(
                    height * 0.18f
                    ) + 82,
                20,
                Color{ 108, 232, 255, 255 }
            );

            DrawRectangle(
                82,
                static_cast<int>(
                    height * 0.18f
                    ) + 118,
                330,
                2,
                Color{ 58, 153, 177, 255 }
            );

            const float buttonWidth = 310.0f;
            const float buttonHeight = 54.0f;
            const float startY =
                height * 0.48f;

            const char* menuItems[] = {
                "NEW GAME",
                "CONTROLS",
                "EXIT"
            };

            for (int i = 0; i < 3; i++)
            {
                const Rectangle button{
                    82.0f,
                    startY +
                    i * 67.0f,
                    buttonWidth,
                    buttonHeight
                };

                const bool clicked =
                    DrawMenuButton(
                        menuItems[i],
                        button,
                        selectedMenuItem == i
                    );

                if (
                    CheckCollisionPointRec(
                        GetMousePosition(),
                        button
                    )
                    )
                {
                    selectedMenuItem = i;
                }

                if (clicked)
                {
                    selectedMenuItem = i;

                    if (i == 0)
                    {
                        storyPage = 0;
                        storyPageTime = 0.0f;
                        applicationScreen =
                            ApplicationScreen::StoryIntro;
                    }
                    else if (i == 1)
                    {
                        applicationScreen =
                            ApplicationScreen::Controls;
                    }
                    else
                    {
                        requestExit = true;
                    }
                }
            }

            if (IsKeyPressed(KEY_ENTER))
            {
                if (
                    selectedMenuItem == 0
                    )
                {
                    storyPage = 0;
                    storyPageTime = 0.0f;
                    applicationScreen =
                        ApplicationScreen::StoryIntro;
                }
                else if (selectedMenuItem == 1)
                {
                    applicationScreen =
                        ApplicationScreen::Controls;
                }
                else
                {
                    requestExit = true;
                }
            }

            DrawText(
                "W/S OR ARROWS TO NAVIGATE   -   ENTER TO SELECT",
                84,
                height - 38,
                15,
                Color{ 112, 142, 154, 255 }
            );

            DrawText(
                "v0.2  -  THE WARDEN AWAKENS",
                width - 276,
                height - 34,
                15,
                Color{ 112, 142, 154, 255 }
            );

            EndDrawing();
            continue;
        }

        if (
            applicationScreen ==
            ApplicationScreen::StoryIntro
            )
        {
            storyPageTime += deltaTime;

            if (IsKeyPressed(KEY_ENTER))
            {
                storyPage++;
                storyPageTime = 0.0f;

                if (storyPage >= 3)
                {
                    applicationScreen =
                        ApplicationScreen::Gameplay;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                applicationScreen =
                    ApplicationScreen::Gameplay;
            }

            BeginDrawing();
            DrawStoryPage(
                storyPage,
                storyPageTime
            );
            EndDrawing();
            continue;
        }

        if (
            applicationScreen ==
            ApplicationScreen::Controls
            )
        {
            if (
                IsKeyPressed(KEY_ESCAPE) ||
                IsKeyPressed(KEY_ENTER)
                )
            {
                applicationScreen =
                    ApplicationScreen::MainMenu;
            }

            BeginDrawing();
            DrawAtmosphericBackground(
                menuAnimationTime
            );

            const int width =
                GetScreenWidth();

            const int height =
                GetScreenHeight();

            DrawRectangle(
                width / 2 - 310,
                height / 2 - 245,
                620,
                490,
                Color{ 5, 15, 27, 235 }
            );

            DrawRectangleLinesEx(
                Rectangle{
                    static_cast<float>(
                        width / 2 - 310
                    ),
                    static_cast<float>(
                        height / 2 - 245
                    ),
                    620.0f,
                    490.0f
                },
                2.0f,
                Color{ 74, 185, 209, 255 }
            );

            const char* controlsTitle =
                "CONTROLS";

            DrawText(
                controlsTitle,
                width / 2 -
                MeasureText(
                    controlsTitle,
                    42
                ) / 2,
                height / 2 - 205,
                42,
                Color{ 108, 232, 255, 255 }
            );

            const char* controlLines[] = {
                "W A S D        Move",
                "LEFT SHIFT    Run",
                "SPACE         Dash",
                "LEFT MOUSE    Attack",
                "E (HOLD)      Activate Altar",
                "B             Open / Close Shop",
                "ESC / ENTER   Return to Menu"
            };

            for (int i = 0; i < 7; i++)
            {
                DrawText(
                    controlLines[i],
                    width / 2 - 190,
                    height / 2 -
                    110 +
                    i * 43,
                    21,
                    i == 6
                    ? GRAY
                    : RAYWHITE
                );
            }

            EndDrawing();
            continue;
        }

        // =====================================================
        // RESTART AFTER GAME OVER
        // =====================================================

        if (
            gameState == GameState::GameOver &&
            IsKeyPressed(KEY_R)
            )
        {
            currentLevel = 1;

            scoreManager.Reset();

            player = Player();

            combatSystem =
                CombatSystem();

            boss =
                Boss(
                    Vector3{
                        0.0f,
                        3.3f,
                        -10.0f
                    }
                );

            bossFightActive = false;
            bossRewardGranted = false;

            aetherCore.Reset();
            defenseCore.Reset(
                Vector3{
                    0.0f,
                    1.25f,
                    -4.0f
                }
            );

            altars.clear();

            altars.emplace_back(
                Vector3{
                    -10.0f,
                    0.5f,
                    -8.0f
                }
            );

            altars.emplace_back(
                Vector3{
                    10.0f,
                    0.5f,
                    -6.0f
                }
            );

            altars.emplace_back(
                Vector3{
                    0.0f,
                    0.5f,
                    10.0f
                }
            );

            exitDoor =
                ExitDoor(
                    Vector3{
                        18.0f,
                        1.5f,
                        0.0f
                    }
                );

            activatedAltars = 0;

            enemyManager.Reset();

            coinManager.Reset();

            coinCount = 0;

            gameState =
                GameState::Playing;

            cameraShakeTime = 0.0f;
            lavaDamageTimer = 0.0f;
            lastHealedDefenseWave = 0;
            levelCompleteAutoTimer = 0.0f;

            camera.position = {
                player.GetPosition().x +
                cameraOffset.x,

                player.GetPosition().y +
                cameraOffset.y,

                player.GetPosition().z +
                cameraOffset.z
            };

            camera.target =
                player.GetPosition();
        }

        // =====================================================
        // START LEVEL 2
        // =====================================================

        if (
            gameState == GameState::LevelComplete &&
            currentLevel == 1 &&
            IsKeyPressed(KEY_ENTER)
            )
        {
            currentLevel = 2;

            // Level 2 is a direct continuation of Level 1. Keep the same
            // Warden, health, armor, speed and shop upgrades.
            player.SetPosition(Vector3{ 0.0f, 0.5f, 4.5f });
            player.SetFacingDirection(Vector3{ 0.0f, 0.0f, -1.0f });

            altars.clear();
            activatedAltars = 0;

            enemyManager.Reset();
            enemyManager.GetEnemies().clear();
            aetherSealbreaker.Start();

            lastHealedDefenseWave = 0;
            levelCompleteAutoTimer = 0.0f;

            coinManager.Reset();

            boss =
                Boss(
                    Vector3{
                        0.0f,
                        3.3f,
                        -10.0f
                    }
                );

            bossFightActive = false;
            bossRewardGranted = false;

            aetherCore.Reset();

            // CombatSystem burada sıfırlanmıyor.
            // Böylece Shop'tan alınan damage upgrade korunur.

            gameState =
                GameState::Playing;

            cameraShakeTime = 0.0f;
            lavaDamageTimer = 0.0f;

            camera.position = {
                player.GetPosition().x +
                cameraOffset.x,

                player.GetPosition().y +
                cameraOffset.y,

                player.GetPosition().z +
                cameraOffset.z
            };

            camera.target =
                player.GetPosition();
        }

        // =====================================================
        // START LEVEL 3 - AETHER TITAN
        // =====================================================

        if (
            gameState == GameState::LevelComplete &&
            currentLevel == 2 &&
            levelCompleteAutoTimer > 0.0f
            )
        {
            levelCompleteAutoTimer -= deltaTime;

            if (levelCompleteAutoTimer < 0.0f)
            {
                levelCompleteAutoTimer = 0.0f;
            }
        }

        if (
            gameState == GameState::LevelComplete &&
            currentLevel == 2 &&
            (
                IsKeyPressed(KEY_ENTER) ||
                levelCompleteAutoTimer <= 0.0f
                )
            )
        {
            currentLevel = 3;

            // Carry the same Warden through the shattered gate.
            player.SetPosition(Vector3{ 0.0f, 0.5f, 5.5f });
            player.SetFacingDirection(Vector3{ 0.0f, 0.0f, -1.0f });

            enemyManager.Reset();
            coinManager.Reset();

            boss =
                Boss(
                    Vector3{
                        0.0f,
                        3.3f,
                        -10.0f
                    }
                );

            bossFightActive = true;
            bossRewardGranted = false;

            aetherCore.Reset();

            gameState =
                GameState::Playing;

            cameraShakeTime = 0.0f;
            lavaDamageTimer = 0.0f;

            camera.position = {
                player.GetPosition().x +
                cameraOffset.x,

                player.GetPosition().y +
                cameraOffset.y,

                player.GetPosition().z +
                cameraOffset.z
            };

            camera.target =
                player.GetPosition();
        }

        // =====================================================
        // UPDATE
        // =====================================================

        if (gameState == GameState::Playing)
        {
            if (currentLevel != 2)
            {
                shop.Update(
                    player,
                    combatSystem,
                    coinCount
                );
            }

            if (!shop.IsOpen())
            {
                if (currentLevel == 2)
                {
                    aetherSealbreaker.Update(deltaTime, player, camera);
                }
                else
                {
                    player.Update(deltaTime);

                    if (bossFightActive)
                    {
                        combatSystem.UpdateBoss(
                            deltaTime,
                            player,
                            boss
                        );
                    }
                    else
                    {
                        enemyManager.Update(
                            deltaTime,
                            player.GetPosition()
                        );

                        combatSystem.Update(
                            deltaTime,
                            player,
                            enemyManager.GetEnemies()
                        );
                    }
                }
            }

            // =================================================
            // NORMAL ENEMY SCORE AND COINS
            // =================================================

            if (!bossFightActive)
            {
                for (
                    Enemy& enemy :
                    enemyManager.GetEnemies()
                    )
                {
                    if (
                        !enemy.IsAlive() &&
                        !enemy.ScoreAlreadyGiven()
                        )
                    {
                        scoreManager.AddScore(
                            enemy.GetScoreValue()
                        );

                        coinManager.SpawnCoin(
                            enemy.GetPosition()
                        );

                        enemy.MarkScoreGiven();
                    }
                }
            }

            coinCount +=
                coinManager.Update(
                    deltaTime,
                    player.GetPosition()
                );

            // Level 3'teki parlak lav havuzlari gercek bir tehlikedir.
            // Hasar aralikli uygulanir; oyuncunun kacmak icin zamani olur.
            if (
                !shop.IsOpen() &&
                currentLevel == 3 &&
                bossFightActive &&
                worldEnvironment.IsHazardAt(
                    currentLevel,
                    player.GetPosition()
                )
                )
            {
                lavaDamageTimer -= deltaTime;

                if (lavaDamageTimer <= 0.0f)
                {
                    player.TakeDamage(7);
                    lavaDamageTimer = 1.10f;
                    cameraShakeTime = 0.10f;
                }
            }
            else
            {
                lavaDamageTimer = 0.0f;
            }

            // =================================================
            // LEVEL OBJECTIVE AND EXIT DOOR
            // =================================================

            if (!bossFightActive)
            {
                if (currentLevel == 1)
                {
                    for (auto& altar : altars)
                    {
                        const bool altarCompleted =
                            altar.Update(
                                deltaTime,
                                player.GetPosition(),
                                !shop.IsOpen() &&
                                IsKeyDown(KEY_E)
                            );

                        if (altarCompleted)
                        {
                            scoreManager.AddScore(500);
                            cameraShakeTime = 0.18f;
                        }
                    }

                    activatedAltars = 0;

                    for (const auto& altar : altars)
                    {
                        if (altar.IsActivated())
                        {
                            activatedAltars++;
                        }
                    }

                    exitDoor.Update(
                        player.GetPosition(),
                        activatedAltars == totalAltars
                    );
                }
                else if (currentLevel == 2)
                {
                    if (aetherSealbreaker.IsComplete())
                    {
                        scoreManager.AddScore(1500);

                        levelCompleteAutoTimer = 4.0f;

                        gameState =
                            GameState::LevelComplete;
                    }
                }

                if (
                    currentLevel == 1 &&
                    exitDoor.PlayerReachedDoor()
                    )
                {
                    gameState =
                        GameState::LevelComplete;
                }
            }

            // =================================================
            // BOSS DEFEATED
            // =================================================

            if (
                bossFightActive &&
                !boss.IsAlive() &&
                !bossRewardGranted
                )
            {
                scoreManager.AddScore(
                    2000
                );

                aetherCore.Spawn(
                    boss.GetPosition()
                );

                bossRewardGranted = true;
            }

            if (
                bossFightActive &&
                aetherCore.IsSpawned()
                )
            {
                aetherCore.Update(
                    deltaTime,
                    player.GetPosition()
                );
            }

            if (
                bossFightActive &&
                aetherCore.IsCollected()
                )
            {
                gameState =
                    GameState::LevelComplete;
            }

            if (
                combatSystem
                .ConsumeCameraShakeRequest()
                )
            {
                cameraShakeTime =
                    cameraShakeDuration;
            }

            if (!player.IsAlive())
            {
                gameState =
                    GameState::GameOver;
            }
        }

        // =====================================================
        // CAMERA FOLLOW
        // =====================================================

        const Vector3 playerPosition =
            player.GetPosition();

        const Vector3 cameraFocus =
            currentLevel == 2
            ? Vector3{
                0.0f,
                3.6f,
                -3.2f
            }
            : playerPosition;

        // The Titan arena uses a wider tactical camera. This keeps both the
        // player, the boss and incoming meteor warnings visible at once.
        const Vector3 activeCameraOffset =
            currentLevel == 2
            ? Vector3{ 0.0f, 4.8f, 20.0f }
            : currentLevel == 3
            ? Vector3{ 0.0f, 12.5f, 15.5f }
            : cameraOffset;

        const Vector3 desiredCameraPosition{
            cameraFocus.x + activeCameraOffset.x,
            cameraFocus.y + activeCameraOffset.y,
            cameraFocus.z + activeCameraOffset.z
        };

        camera.target =
            Vector3Lerp(
                camera.target,
                cameraFocus,
                cameraFollowSpeed * deltaTime
            );

        camera.position =
            Vector3Lerp(
                camera.position,
                desiredCameraPosition,
                cameraFollowSpeed * deltaTime
            );

        // =====================================================
        // CAMERA SHAKE
        // =====================================================

        if (cameraShakeTime > 0.0f)
        {
            cameraShakeTime -= deltaTime;

            camera.position.x +=
                GetRandomValue(
                    -100,
                    100
                ) /
                100.0f *
                cameraShakeStrength;

            camera.position.y +=
                GetRandomValue(
                    -100,
                    100
                ) /
                100.0f *
                cameraShakeStrength;

            if (cameraShakeTime < 0.0f)
            {
                cameraShakeTime = 0.0f;
            }
        }

        // =====================================================
        // DRAW
        // =====================================================

        BeginDrawing();

        ClearBackground(
            currentLevel == 1
            ? Color{ 9, 13, 23, 255 }
            : currentLevel == 2
            ? Color{ 7, 24, 18, 255 }
            : Color{ 28, 8, 5, 255 }
        );

        if (currentLevel == 2)
        {
            BeginMode3D(camera);

            worldEnvironment.Draw(currentLevel, false);
            aetherSealbreaker.DrawWorld(player);
            player.Draw();

            EndMode3D();
            aetherSealbreaker.DrawOverlay();
        }
        else
        {
            BeginMode3D(
                camera
            );

            worldEnvironment.Draw(
                currentLevel,
                bossFightActive
            );

            player.Draw();

            if (bossFightActive)
            {
                boss.Draw();
                aetherCore.Draw();
            }
            else
            {
                enemyManager.Draw();

                if (currentLevel == 1)
                {
                    for (const auto& altar : altars)
                    {
                        altar.Draw();
                    }

                    exitDoor.Draw();
                }
            }

            coinManager.Draw();
            EndMode3D();
        }

        if (!bossFightActive && currentLevel != 2)
        {
            for (
                const Enemy& enemy :
                enemyManager.GetEnemies()
                )
            {
                UI::DrawEnemyHealthBar(
                    enemy,
                    camera
                );
            }
        }

        if (currentLevel != 2)
        {
            UI::DrawHUD(
                player,
                currentLevel == 1 ? "Altars" : "Titan",
                currentLevel == 1
                    ? activatedAltars
                    : boss.IsAlive() ? 0 : 1,
                currentLevel == 1 ? totalAltars : 1
            );
        }

        if (
            currentLevel == 1 &&
            gameState == GameState::Playing &&
            !shop.IsOpen()
            )
        {
            for (const auto& altar : altars)
            {
                if (altar.IsPlayerInRange())
                {
                    UI::DrawAltarInteraction(
                        altar.GetActivationProgress()
                    );
                    break;
                }
            }
        }

        if (currentLevel != 2)
        {
            UI::DrawScore(
                scoreManager.GetScore()
            );

            UI::DrawCoins(
                coinCount
            );
        }

        if (
            !bossFightActive &&
            currentLevel != 2 &&
            gameState == GameState::Playing
            )
        {
            UI::DrawWaveInformation(
                enemyManager
            );
        }

        // =====================================================
        // PROFESSIONAL BOSS HUD
        // =====================================================

        if (
            bossFightActive &&
            boss.IsAlive()
            )
        {
            BossHUD::Draw(
                boss
            );
        }

        if (
            bossFightActive &&
            aetherCore.IsSpawned()
            )
        {
            const char* coreText =
                "Collect the Aether Core!";

            const int coreFontSize = 28;

            const int coreTextWidth =
                MeasureText(
                    coreText,
                    coreFontSize
                );

            DrawText(
                coreText,
                GetScreenWidth() / 2 -
                coreTextWidth / 2,
                70,
                coreFontSize,
                SKYBLUE
            );
        }

        // =====================================================
        // LEVEL NUMBER
        // =====================================================

        if (currentLevel != 2)
        {
            const char* levelText =
                TextFormat(
                    "LEVEL %d",
                    currentLevel
                );

            const int levelFontSize = 26;

            const int levelTextWidth =
                MeasureText(
                    levelText,
                    levelFontSize
                );

            DrawText(
                levelText,
                GetScreenWidth() / 2 -
                levelTextWidth / 2,
                20,
                levelFontSize,
                WHITE
            );

            const char* missionText =
                currentLevel == 1
                ? activatedAltars == totalAltars
                    ? "THE GATE IS OPEN - REACH THE EXIT"
                    : "AWAKEN THE RUINS - ACTIVATE 3 ALTARS"
                : bossFightActive
                    ? boss.IsCoreExposed()
                        ? "CORE EXPOSED - ATTACK THE TITAN NOW"
                        : "DODGE 2 METEORS - EXPOSE THE CORE - STRIKE"
                    : "ENTER THE TITAN ARENA";

            const int missionFontSize = 18;

            DrawText(
                missionText,
                GetScreenWidth() / 2 -
                MeasureText(
                    missionText,
                    missionFontSize
                ) / 2,
                52,
                missionFontSize,
                currentLevel == 1 ? SKYBLUE : GOLD
            );
        }

        // =====================================================
        // GAME OVER
        // =====================================================

        if (gameState == GameState::GameOver)
        {
            DrawRectangle(
                0,
                0,
                GetScreenWidth(),
                GetScreenHeight(),
                Color{
                    0,
                    0,
                    0,
                    150
                }
            );

            const char* gameOverText =
                "GAME OVER";

            const int gameOverFontSize = 60;

            const int gameOverTextWidth =
                MeasureText(
                    gameOverText,
                    gameOverFontSize
                );

            DrawText(
                gameOverText,
                GetScreenWidth() / 2 -
                gameOverTextWidth / 2,
                GetScreenHeight() / 2 -
                60,
                gameOverFontSize,
                RED
            );

            const char* restartText =
                "Press R to Restart";

            const int restartFontSize = 24;

            const int restartTextWidth =
                MeasureText(
                    restartText,
                    restartFontSize
                );

            DrawText(
                restartText,
                GetScreenWidth() / 2 -
                restartTextWidth / 2,
                GetScreenHeight() / 2 +
                20,
                restartFontSize,
                WHITE
            );
        }

        // =====================================================
        // LEVEL COMPLETE / GAME COMPLETE
        // =====================================================

        if (gameState == GameState::LevelComplete)
        {
            DrawRectangle(
                0,
                0,
                GetScreenWidth(),
                GetScreenHeight(),
                Color{
                    0,
                    0,
                    0,
                    170
                }
            );

            const char* completeText =
                currentLevel < 3
                ? "LEVEL COMPLETE"
                : "GAME COMPLETE";

            const int completeFontSize = 60;

            const int completeWidth =
                MeasureText(
                    completeText,
                    completeFontSize
                );

            DrawText(
                completeText,
                GetScreenWidth() / 2 -
                completeWidth / 2,
                GetScreenHeight() / 2 -
                70,
                completeFontSize,
                GREEN
            );

            const char* continueText =
                currentLevel == 1
                ? "Press ENTER to Start Level 2"
                : currentLevel == 2
                ? TextFormat(
                    "SHATTERED GATE CROSSED - LEVEL 3 STARTS IN %d",
                    static_cast<int>(
                        ceilf(levelCompleteAutoTimer)
                    )
                )
                : "You Defeated the Aether Titan!";

            const int continueFontSize = 26;

            const int continueWidth =
                MeasureText(
                    continueText,
                    continueFontSize
                );

            DrawText(
                continueText,
                GetScreenWidth() / 2 -
                continueWidth / 2,
                GetScreenHeight() / 2 +
                20,
                continueFontSize,
                WHITE
            );
        }

        shop.Draw(
            player,
            combatSystem,
            coinCount
        );

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
