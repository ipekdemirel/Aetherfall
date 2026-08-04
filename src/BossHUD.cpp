#include "BossHUD.h"

#include "Boss.h"

#include <raylib.h>

#include <algorithm>
#include <cmath>

namespace
{
    float Clamp01(float value)
    {
        return std::clamp(value, 0.0f, 1.0f);
    }

    Color GetBossHealthColor(
        float healthRatio,
        bool rageModeActive
    )
    {
        if (rageModeActive)
        {
            return Color{
                255,
                65,
                35,
                255
            };
        }

        if (healthRatio > 0.60f)
        {
            return Color{
                185,
                35,
                55,
                255
            };
        }

        if (healthRatio > 0.30f)
        {
            return Color{
                235,
                120,
                30,
                255
            };
        }

        return Color{
            235,
            45,
            35,
            255
        };
    }

    void DrawCenteredText(
        const char* text,
        int centerX,
        int y,
        int fontSize,
        Color color
    )
    {
        const int textWidth =
            MeasureText(
                text,
                fontSize
            );

        DrawText(
            text,
            centerX - textWidth / 2,
            y,
            fontSize,
            color
        );
    }
}

void BossHUD::Draw(const Boss& boss)
{
    if (!boss.IsAlive())
    {
        return;
    }

    const int screenWidth =
        GetScreenWidth();

    const int screenHeight =
        GetScreenHeight();

    const float currentHealth =
        boss.GetHealth();

    const float maximumHealth =
        boss.GetMaxHealth();

    float healthRatio = 0.0f;

    if (maximumHealth > 0.0f)
    {
        healthRatio =
            Clamp01(
                currentHealth /
                maximumHealth
            );
    }

    const bool rageModeActive =
        boss.IsRageModeActive();

    // Compact boss HUD: it leaves the arena and the Titan visible.
    int panelWidth =
        static_cast<int>(
            screenWidth * 0.46f
            );

    panelWidth =
        std::clamp(
            panelWidth,
            420,
            700
        );

    const int panelHeight = 92;

    const int panelX =
        screenWidth / 2 -
        panelWidth / 2;

    const int panelY =
        std::max(
            65,
            static_cast<int>(
                screenHeight * 0.075f
                )
        );

    // =====================================================
    // PANEL SHADOW
    // =====================================================

    DrawRectangleRounded(
        Rectangle{
            static_cast<float>(
                panelX + 6
            ),
            static_cast<float>(
                panelY + 7
            ),
            static_cast<float>(
                panelWidth
            ),
            static_cast<float>(
                panelHeight
            )
        },
        0.18f,
        12,
        Color{
            0,
            0,
            0,
            105
        }
    );

    // =====================================================
    // MAIN PANEL
    // =====================================================

    DrawRectangleRounded(
        Rectangle{
            static_cast<float>(
                panelX
            ),
            static_cast<float>(
                panelY
            ),
            static_cast<float>(
                panelWidth
            ),
            static_cast<float>(
                panelHeight
            )
        },
        0.18f,
        12,
        Color{
            15,
            12,
            24,
            225
        }
    );

    // Dış altın çerçeve
    DrawRectangleRoundedLinesEx(
        Rectangle{
            static_cast<float>(
                panelX
            ),
            static_cast<float>(
                panelY
            ),
            static_cast<float>(
                panelWidth
            ),
            static_cast<float>(
                panelHeight
            )
        },
        0.18f,
        12,
        3.0f,
        Color{
            213,
            169,
            69,
            255
        }
    );

    // İç çerçeve
    DrawRectangleRoundedLinesEx(
        Rectangle{
            static_cast<float>(
                panelX + 5
            ),
            static_cast<float>(
                panelY + 5
            ),
            static_cast<float>(
                panelWidth - 10
            ),
            static_cast<float>(
                panelHeight - 10
            )
        },
        0.15f,
        12,
        1.0f,
        Color{
            100,
            74,
            35,
            190
        }
    );

    // =====================================================
    // BOSS TITLE
    // =====================================================

    const char* bossName =
        rageModeActive
        ? "AETHER TITAN  -  RAGE MODE"
        : "AETHER TITAN";

    const Color bossTitleColor =
        rageModeActive
        ? Color{
            255,
            105,
            65,
            255
    }
        : Color{
            237,
            211,
            138,
            255
    };

    DrawCenteredText(
        bossName,
        screenWidth / 2,
        panelY + 8,
        rageModeActive
        ? 18
        : 20,
        bossTitleColor
    );

    // Boss adının iki yanındaki süs çizgileri
    const int titleLineY =
        panelY + 19;

    DrawLineEx(
        Vector2{
            static_cast<float>(
                panelX + 28
            ),
            static_cast<float>(
                titleLineY
            )
        },
        Vector2{
            static_cast<float>(
                panelX + 118
            ),
            static_cast<float>(
                titleLineY
            )
        },
        2.0f,
        Color{
            146,
            107,
            46,
            210
        }
    );

    DrawLineEx(
        Vector2{
            static_cast<float>(
                panelX + panelWidth - 118
            ),
            static_cast<float>(
                titleLineY
            )
        },
        Vector2{
            static_cast<float>(
                panelX + panelWidth - 28
            ),
            static_cast<float>(
                titleLineY
            )
        },
        2.0f,
        Color{
            146,
            107,
            46,
            210
        }
    );

    // =====================================================
    // HEALTH BAR
    // =====================================================

    const int barX =
        panelX + 34;

    const int barY =
        panelY + 34;

    const int barWidth =
        panelWidth - 68;

    const int barHeight = 22;

    // Bar dış gölgesi
    DrawRectangleRounded(
        Rectangle{
            static_cast<float>(
                barX + 3
            ),
            static_cast<float>(
                barY + 4
            ),
            static_cast<float>(
                barWidth
            ),
            static_cast<float>(
                barHeight
            )
        },
        0.30f,
        10,
        Color{
            0,
            0,
            0,
            135
        }
    );

    // Bar arka planı
    DrawRectangleRounded(
        Rectangle{
            static_cast<float>(
                barX
            ),
            static_cast<float>(
                barY
            ),
            static_cast<float>(
                barWidth
            ),
            static_cast<float>(
                barHeight
            )
        },
        0.30f,
        10,
        Color{
            37,
            30,
            42,
            255
        }
    );

    const float healthFillWidth =
        static_cast<float>(
            barWidth - 6
            ) *
        healthRatio;

    const Color healthColor =
        GetBossHealthColor(
            healthRatio,
            rageModeActive
        );

    // Dolu can kısmı
    if (healthFillWidth > 1.0f)
    {
        DrawRectangleRounded(
            Rectangle{
                static_cast<float>(
                    barX + 3
                ),
                static_cast<float>(
                    barY + 3
                ),
                healthFillWidth,
                static_cast<float>(
                    barHeight - 6
                )
            },
            0.28f,
            10,
            healthColor
        );

        // Barın üstündeki ışık katmanı
        DrawRectangleRounded(
            Rectangle{
                static_cast<float>(
                    barX + 5
                ),
                static_cast<float>(
                    barY + 5
                ),
                std::max(
                    0.0f,
                    healthFillWidth - 4.0f
                ),
                7.0f
            },
            0.35f,
            8,
            Color{
                255,
                255,
                255,
                52
            }
        );
    }

    // Bar çerçevesi
    DrawRectangleRoundedLinesEx(
        Rectangle{
            static_cast<float>(
                barX
            ),
            static_cast<float>(
                barY
            ),
            static_cast<float>(
                barWidth
            ),
            static_cast<float>(
                barHeight
            )
        },
        0.30f,
        10,
        2.0f,
        Color{
            226,
            190,
            104,
            255
        }
    );

    // Barın içindeki bölme çizgileri
    constexpr int sectionCount = 10;

    for (
        int section = 1;
        section < sectionCount;
        ++section
        )
    {
        const float lineX =
            static_cast<float>(
                barX
                ) +
            static_cast<float>(
                barWidth
                ) *
            (
                static_cast<float>(
                    section
                    ) /
                static_cast<float>(
                    sectionCount
                    )
                );

        DrawLineEx(
            Vector2{
                lineX,
                static_cast<float>(
                    barY + 4
                )
            },
            Vector2{
                lineX,
                static_cast<float>(
                    barY +
                    barHeight -
                    4
                )
            },
            1.0f,
            Color{
                15,
                10,
                19,
                95
            }
        );
    }

    // Canın bittiği noktadaki parlama
    if (
        healthRatio > 0.02f &&
        healthRatio < 0.995f
        )
    {
        const float pulse =
            (
                std::sin(
                    static_cast<float>(
                        GetTime()
                        ) *
                    7.0f
                ) +
                1.0f
                ) *
            0.5f;

        const float glowX =
            static_cast<float>(
                barX + 3
                ) +
            healthFillWidth;

        DrawCircleGradient(
            static_cast<int>(
                glowX
                ),
            barY + barHeight / 2,
            9.0f + pulse * 3.0f,
            Color{
                255,
                225,
                175,
                static_cast<unsigned char>(
                    150 +
                    pulse * 70.0f
                )
            },
            Color{
                255,
                80,
                40,
                0
            }
        );
    }

    // =====================================================
    // HEALTH NUMBER
    // =====================================================

    const char* healthText =
        TextFormat(
            "%d / %d",
            static_cast<int>(
                currentHealth
                ),
            static_cast<int>(
                maximumHealth
                )
        );

    const int healthTextSize = 15;

    const int healthTextWidth =
        MeasureText(
            healthText,
            healthTextSize
        );

    // Yazının gölgesi
    DrawText(
        healthText,
        screenWidth / 2 -
        healthTextWidth / 2 +
        2,
        barY + 3,
        healthTextSize,
        Color{
            0,
            0,
            0,
            190
        }
    );

    DrawText(
        healthText,
        screenWidth / 2 -
        healthTextWidth / 2,
        barY + 1,
        healthTextSize,
        RAYWHITE
    );

    // =====================================================
    // PHASE / STATUS TEXT
    // =====================================================

    const char* statusText = nullptr;

    Color statusColor = Color{ 174, 163, 190, 255 };

    if (boss.IsCoreExposed())
    {
        statusText = "CORE EXPOSED - ATTACK NOW!";
        statusColor = Color{ 92, 238, 255, 255 };
    }
    else if (boss.GetDodgedMeteorCount() > 0)
    {
        statusText = TextFormat(
            "DODGE METEORS  %d / %d  -  BREAK ITS ARMOR",
            boss.GetDodgedMeteorCount(),
            boss.GetMeteorsNeededToExposeCore()
        );
        statusColor = GOLD;
    }
    else if (rageModeActive)
    {
        statusText =
            "PHASE II  -  ENRAGED";
    }
    else if (healthRatio <= 0.75f)
    {
        statusText =
            "PHASE I  -  AWAKENED";
    }
    else
    {
        statusText =
            "THE FALLEN GUARDIAN";
    }

    DrawCenteredText(
        statusText,
        screenWidth / 2,
        panelY + 64,
        14,
        statusColor
    );
}
