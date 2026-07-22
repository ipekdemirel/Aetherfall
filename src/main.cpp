#include <raylib.h>
#include <raymath.h>

#include "Player.h"

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

    SetTargetFPS(60);

    Player player;

    Camera3D camera{};
    camera.position = { 0.0f, 10.0f, 12.0f };
    camera.target = player.GetPosition();
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const Vector3 cameraOffset{
        0.0f,
        10.0f,
        12.0f
    };

    const float cameraFollowSpeed = 7.0f;

    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        player.Update(deltaTime);

        const Vector3 playerPosition = player.GetPosition();

        const Vector3 desiredCameraTarget{
            playerPosition.x,
            playerPosition.y,
            playerPosition.z
        };

        const Vector3 desiredCameraPosition{
            playerPosition.x + cameraOffset.x,
            playerPosition.y + cameraOffset.y,
            playerPosition.z + cameraOffset.z
        };

        camera.target = Vector3Lerp(
            camera.target,
            desiredCameraTarget,
            cameraFollowSpeed * deltaTime
        );

        camera.position = Vector3Lerp(
            camera.position,
            desiredCameraPosition,
            cameraFollowSpeed * deltaTime
        );

        BeginDrawing();

        ClearBackground(Color{ 135, 206, 235, 255 });

        BeginMode3D(camera);

        DrawPlane(
            { 0.0f, 0.0f, 0.0f },
            { 50.0f, 50.0f },
            DARKGREEN
        );

        DrawGrid(50, 1.0f);

        player.Draw();

        EndMode3D();

        DrawText(
            "AETHERFALL",
            20,
            20,
            28,
            BLACK
        );

        DrawText(
            "WASD: Move",
            20,
            58,
            20,
            DARKGRAY
        );

        DrawText(
            "LEFT SHIFT: Run",
            20,
            86,
            20,
            DARKGRAY
        );

        DrawText(
            "SPACE: Dash",
            20,
            114,
            20,
            DARKGRAY
        );

        DrawFPS(
            GetScreenWidth() - 100,
            20
        );

        EndDrawing();
    }

    CloseWindow();

    return 0;
}