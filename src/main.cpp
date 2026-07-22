#include <raylib.h>

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(screenWidth, screenHeight, "Aetherfall");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(BLACK);

        DrawRectangleGradientV(
            0,
            0,
            GetScreenWidth(),
            GetScreenHeight(),
            Color{ 18, 20, 35, 255 },
            Color{ 45, 25, 70, 255 }
        );

        DrawText(
            "AETHERFALL",
            GetScreenWidth() / 2 - 170,
            GetScreenHeight() / 2 - 70,
            60,
            RAYWHITE
        );

        DrawText(
            "Initial Project Setup",
            GetScreenWidth() / 2 - 120,
            GetScreenHeight() / 2 + 10,
            24,
            SKYBLUE
        );

        EndDrawing();
    }

    CloseWindow();

    return 0;
}