#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <queue>
#include <vector>

#include <raylib.h>
#include <raymath.h>

#include "Player.h"

class AetherSealbreaker
{
public:
    AetherSealbreaker()
    {
        InitializeFeedbackAudio();
        Start();
    }

    ~AetherSealbreaker()
    {
        ShutdownFeedbackAudio();
    }

    AetherSealbreaker(const AetherSealbreaker&) = delete;
    AetherSealbreaker& operator=(const AetherSealbreaker&) = delete;

    void Start()
    {
        for (auto& row : grid)
        {
            for (Cell& cell : row)
            {
                cell.active = false;
                cell.color = 0;
                cell.armor = 0;
                cell.corrupted = false;
            }
        }

        score = 0;
        shotsFired = 0;
        poppedCount = 0;
        sealsBroken = 0;
        combo = 0;
        missesUntilPressure = GetMissLimit();
        power = 0;
        playerHealth = MaxPlayerHealth;
        playerX = GetScreenWidth() * 0.5f;
        launcherPosition = Vector2{
            playerX,
            GetScreenHeight() - 135.0f
        };
        portalWalk = 0.0f;
        phaseTimer = 0.0f;
        pressureTimer = GetPressureTime();
        coreShiftTimer = PhaseThreeCoreShiftTime;
        introTimer = 4.5f;
        projectileActive = false;
        portalOpen = false;
        completed = false;
        failed = false;
        message = Message::Intro;
        messageTimer = 4.0f;
        screenFlash = 0.0f;
        shakeTimer = 0.0f;
        hitFeedbackTimer = 0.0f;
        hitFeedbackSuccess = true;
        hitFeedbackColor = Color{ 83, 235, 255, 255 };
        impactPosition = Vector2{ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.4f };
        particles.clear();

        BuildOpeningBoard();
        currentColor = PickAvailableColor();
        nextColor = PickAvailableColor();
    }

    void Update(float deltaTime, Player& player, const Camera3D& camera)
    {
        challengeCamera = camera;
        UpdateParticles(deltaTime);
        messageTimer = std::max(0.0f, messageTimer - deltaTime);
        screenFlash = std::max(0.0f, screenFlash - deltaTime);
        shakeTimer = std::max(0.0f, shakeTimer - deltaTime);
        hitFeedbackTimer = std::max(0.0f, hitFeedbackTimer - deltaTime);
        introTimer = std::max(0.0f, introTimer - deltaTime);

        if (!portalOpen)
        {
            const bool playerMoving = UpdatePlayerMovement(deltaTime);
            SyncRealPlayer(player, camera, deltaTime, playerMoving);
        }

        if (completed)
        {
            return;
        }

        if (portalOpen)
        {
            const float gateX = GetScreenWidth() * 0.5f;
            const float distanceToGate = gateX - playerX;

            // The third seal starts a short in-world transition. The player
            // visibly walks into the gate, so there is no hidden position
            // threshold or extra key press after completing the level.
            if (std::fabs(distanceToGate) > 3.0f)
            {
                const float step = 430.0f * deltaTime;
                playerX += std::clamp(distanceToGate, -step, step);
            }

            else
            {
                playerX = gateX;
                portalWalk = std::min(1.0f, portalWalk + deltaTime * 0.62f);
            }

            SyncRealPlayer(player, camera, deltaTime, true);

            if (IsKeyPressed(KEY_ENTER))
            {
                portalWalk = 1.0f;
            }

            if (portalWalk >= 1.0f)
            {
                completed = true;
            }
            return;
        }

        if (failed)
        {
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER))
            {
                Start();
            }
            return;
        }

        if (!projectileActive && (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_Q)))
        {
            std::swap(currentColor, nextColor);
            message = Message::CoreSwapped;
            messageTimer = 0.75f;
        }

        if (!projectileActive && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && introTimer <= 0.0f)
        {
            FireProjectile();
        }

        if (projectileActive)
        {
            UpdateProjectile(deltaTime);
        }

        if (sealsBroken == 2)
        {
            phaseTimer += deltaTime;
            pressureTimer -= deltaTime;
            coreShiftTimer -= deltaTime;

            if (coreShiftTimer <= 0.0f && !projectileActive)
            {
                currentColor = PickAvailableColor();
                nextColor = PickAvailableColor();
                coreShiftTimer = PhaseThreeCoreShiftTime;
                message = Message::CorruptionShift;
                messageTimer = 1.45f;
                TriggerHitFeedback(
                    GetLauncherPosition(),
                    Color{ 196, 91, 255, 255 },
                    false,
                    0.22f
                );
            }

            if (pressureTimer <= 0.0f && !projectileActive)
            {
                AddPressureRow();
                pressureTimer = GetPressureTime();
                if (HasReachedDangerLine())
                {
                    TakeCorruptionHit();
                }
            }
        }
    }

    void DrawWorld(const Player& player) const
    {
        DrawGateArchitecture();
        DrawSealMonoliths();

        if (!portalOpen)
        {
            DrawCrystalWall3D();
            DrawProjectile3D(player);
        }
        else
        {
            DrawPortal3D();
        }

        // Keep the Warrior readable and unobstructed.  The former V9 aura
        // rendered as a large translucent beam/sphere over the character.
    }

    void DrawOverlay() const
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();

        const Rectangle board = GetBoardRectangle();
        if (!portalOpen)
        {
            const float dangerY = CellPosition(Rows - 2, 0).y + OrbRadius;
            DrawLineEx(Vector2{ board.x + 12.0f, dangerY }, Vector2{ board.x + board.width - 12.0f, dangerY }, 2.0f, Color{ 255, 80, 90, 150 });
            DrawText("CORRUPTION FRONT", static_cast<int>(board.x + 15.0f), static_cast<int>(dangerY - 22.0f), 14, Color{ 255, 105, 115, 210 });
            if (!failed)
            {
                DrawAimGuide();
            }
        }

        DrawParticles();

        DrawHUD();

        if (failed)
        {
            DrawFailurePanel();
        }

        if (introTimer > 0.0f && !failed)
        {
            DrawIntroPanel();
        }

        if (screenFlash > 0.0f)
        {
            DrawRectangle(
                0,
                0,
                width,
                height,
                Fade(hitFeedbackColor, screenFlash * 0.28f)
            );
        }
    }

    bool IsComplete() const
    {
        return completed;
    }

private:
    static constexpr int Rows = 12;
    static constexpr int Cols = 12;
    static constexpr int ColorCount = 5;
    static constexpr int MaxPlayerHealth = 3;
    static constexpr int SealCount = 3;
    static constexpr int MissesBeforePressure = 5;
    static constexpr float OrbRadius = 24.0f;
    static constexpr float ProjectileSpeed = 760.0f;
    static constexpr float PhaseThreePressureTime = 7.5f;
    static constexpr float PhaseThreeCoreShiftTime = 6.0f;

    struct Cell
    {
        bool active = false;
        int color = 0;
        int armor = 0;
        bool corrupted = false;
    };

    struct Particle
    {
        Vector2 position{};
        Vector2 velocity{};
        Color color{};
        float life = 0.0f;
        float radius = 0.0f;
    };

    enum class Message
    {
        None,
        Intro,
        Match,
        BigDrop,
        Miss,
        Pressure,
        PrismReady,
        SealBroken,
        CoreSwapped,
        PlayerHit,
        CorruptionShift
    };

    std::array<std::array<Cell, Cols>, Rows> grid{};
    std::vector<Particle> particles;

    int score = 0;
    int shotsFired = 0;
    int poppedCount = 0;
    int sealsBroken = 0;
    int combo = 0;
    int missesUntilPressure = MissesBeforePressure;
    int power = 0;
    int playerHealth = MaxPlayerHealth;

    int currentColor = 0;
    int nextColor = 1;
    bool projectileIsPrism = false;
    bool projectileActive = false;
    Vector2 projectilePosition{};
    Vector2 projectileVelocity{};

    bool portalOpen = false;
    bool completed = false;
    bool failed = false;
    Message message = Message::None;
    float messageTimer = 0.0f;
    float screenFlash = 0.0f;
    float shakeTimer = 0.0f;
    float playerX = 0.0f;
    float portalWalk = 0.0f;
    float phaseTimer = 0.0f;
    float pressureTimer = PhaseThreePressureTime;
    float coreShiftTimer = PhaseThreeCoreShiftTime;
    float introTimer = 0.0f;
    float hitFeedbackTimer = 0.0f;
    bool hitFeedbackSuccess = true;
    Color hitFeedbackColor{ 83, 235, 255, 255 };
    Vector2 impactPosition{};
    Vector2 launcherPosition{};
    Camera3D challengeCamera{};

    Sound matchSound{};
    Sound missSound{};
    Sound sealSound{};
    bool feedbackAudioReady = false;
    bool ownsAudioDevice = false;

    Rectangle GetBoardRectangle() const
    {
        const float boardWidth = Cols * OrbRadius * 2.0f + OrbRadius + 34.0f;
        const float boardHeight = 590.0f;
        return Rectangle{
            GetScreenWidth() / 2.0f - boardWidth / 2.0f,
            108.0f,
            boardWidth,
            std::min(boardHeight, GetScreenHeight() - 220.0f)
        };
    }

    Vector2 GetLauncherPosition() const
    {
        if (launcherPosition.x > 1.0f && launcherPosition.y > 1.0f)
        {
            return launcherPosition;
        }

        const Rectangle board = GetBoardRectangle();
        return Vector2{ playerX, board.y + board.height - 34.0f };
    }

    int GetPhaseTarget() const
    {
        static constexpr int targets[SealCount] = { 900, 1150, 1350 };
        return targets[std::clamp(sealsBroken, 0, SealCount - 1)];
    }

    int GetMissLimit() const
    {
        static constexpr int limits[SealCount] = { 6, 4, 3 };
        return limits[std::clamp(sealsBroken, 0, SealCount - 1)];
    }

    float GetPressureTime() const
    {
        return sealsBroken == 2 ? PhaseThreePressureTime : 99.0f;
    }

    const char* GetPhaseName() const
    {
        static constexpr const char* names[SealCount] = {
            "RESONANCE SEAL",
            "ARMORED SEAL",
            "CORRUPTED SEAL"
        };
        return names[std::clamp(sealsBroken, 0, SealCount - 1)];
    }

    bool UpdatePlayerMovement(float deltaTime)
    {
        float movement = 0.0f;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) movement -= 1.0f;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) movement += 1.0f;
        playerX += movement * 390.0f * deltaTime;
        const Rectangle board = GetBoardRectangle();
        playerX = std::clamp(playerX, board.x + 58.0f, board.x + board.width - 58.0f);
        return std::fabs(movement) > 0.01f;
    }

    float PlayerWorldX() const
    {
        const Rectangle board = GetBoardRectangle();
        const float normalized =
            (playerX - (board.x + board.width * 0.5f)) /
            (board.width * 0.5f);
        return std::clamp(normalized * 7.0f, -7.0f, 7.0f);
    }

    void SyncRealPlayer(
        Player& player,
        const Camera3D& camera,
        float deltaTime,
        bool moving
    )
    {
        const float walkIntoGate = portalOpen ? portalWalk * 11.0f : 0.0f;
        const Vector3 position{
            PlayerWorldX(),
            0.5f,
            4.5f - walkIntoGate
        };

        player.SetPosition(position);
        player.SetFacingDirection(Vector3{ 0.0f, 0.0f, -1.0f });
        player.UpdateChallengeAnimation(deltaTime, moving || portalOpen);

        launcherPosition = GetWorldToScreen(
            Vector3{ position.x + 0.28f, position.y + 1.75f, position.z - 0.35f },
            camera
        );
    }

    Vector2 CellPosition(int row, int col) const
    {
        const Rectangle board = GetBoardRectangle();
        return Vector2{
            board.x + 28.0f + OrbRadius +
                col * OrbRadius * 2.0f +
                (row % 2 == 1 ? OrbRadius : 0.0f),
            board.y + 18.0f + OrbRadius + row * OrbRadius * 1.73f
        };
    }

    Rectangle GetPortalRectangle() const
    {
        const float width = 330.0f;
        const float height = 132.0f;
        return Rectangle{
            GetScreenWidth() / 2.0f - width / 2.0f,
            GetScreenHeight() / 2.0f - height / 2.0f + 35.0f,
            width,
            height
        };
    }

    static float Distance(Vector2 a, Vector2 b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    static Vector2 Normalize(Vector2 value)
    {
        const float length = std::sqrt(value.x * value.x + value.y * value.y);
        if (length < 0.001f)
        {
            return Vector2{ 0.0f, -1.0f };
        }
        return Vector2{ value.x / length, value.y / length };
    }

    Color BubbleColor(int index) const
    {
        static constexpr Color colors[ColorCount] = {
            Color{ 64, 191, 255, 255 },   // Frost Aether
            Color{ 145, 77, 255, 255 },   // Void Aether
            Color{ 255, 126, 42, 255 },   // Ember Aether
            Color{ 67, 207, 139, 255 },   // Ancient Aether
            Color{ 236, 196, 91, 255 }    // Solar Aether
        };
        return colors[index % ColorCount];
    }

    void BuildOpeningBoard()
    {
        for (auto& row : grid)
        {
            for (Cell& cell : row)
            {
                cell = Cell{};
            }
        }

        for (int row = 0; row < 5; ++row)
        {
            const int cellsInRow = row % 2 == 0 ? Cols : Cols - 1;
            for (int col = 0; col < cellsInRow; ++col)
            {
                grid[row][col].active = true;
                grid[row][col].color = (col / 2 + row * 2 + (col % 3 == 0 ? 1 : 0)) % ColorCount;

                if (sealsBroken == 1 && row >= 1 && ((row + col) % 5 == 0))
                {
                    grid[row][col].armor = 1;
                }

                if (sealsBroken == 2 && row >= 1 && ((row * 3 + col) % 7 == 0))
                {
                    grid[row][col].corrupted = true;
                    grid[row][col].armor = 0;
                }
            }
        }
    }

    int PickAvailableColor() const
    {
        std::array<bool, ColorCount> available{};
        int count = 0;
        for (const auto& row : grid)
        {
            for (const Cell& cell : row)
            {
                if (cell.active && !cell.corrupted && !available[cell.color])
                {
                    available[cell.color] = true;
                    ++count;
                }
            }
        }

        if (count == 0)
        {
            return GetRandomValue(0, ColorCount - 1);
        }

        int choice = GetRandomValue(0, count - 1);
        for (int color = 0; color < ColorCount; ++color)
        {
            if (available[color])
            {
                if (choice == 0)
                {
                    return color;
                }
                --choice;
            }
        }
        return 0;
    }

    void FireProjectile()
    {
        Vector2 aim = GetMousePosition();
        const Vector2 launcher = GetLauncherPosition();
        aim.y = std::min(aim.y, launcher.y - 80.0f);
        Vector2 direction = Normalize(Vector2{ aim.x - launcher.x, aim.y - launcher.y });

        if (direction.y > -0.18f)
        {
            direction.y = -0.18f;
            direction = Normalize(direction);
        }

        projectilePosition = launcher;
        projectileVelocity = Vector2{
            direction.x * ProjectileSpeed,
            direction.y * ProjectileSpeed
        };
        projectileActive = true;
        projectileIsPrism = power >= 3;
        if (projectileIsPrism)
        {
            power = 0;
        }
        ++shotsFired;
    }

    void UpdateProjectile(float deltaTime)
    {
        const Rectangle board = GetBoardRectangle();
        projectilePosition.x += projectileVelocity.x * deltaTime;
        projectilePosition.y += projectileVelocity.y * deltaTime;

        const float left = board.x + OrbRadius + 8.0f;
        const float right = board.x + board.width - OrbRadius - 8.0f;

        if (projectilePosition.x <= left)
        {
            projectilePosition.x = left;
            projectileVelocity.x = std::fabs(projectileVelocity.x);
        }
        else if (projectilePosition.x >= right)
        {
            projectilePosition.x = right;
            projectileVelocity.x = -std::fabs(projectileVelocity.x);
        }

        bool collided = projectilePosition.y <= board.y + OrbRadius + 14.0f;
        if (!collided)
        {
            for (int row = 0; row < Rows && !collided; ++row)
            {
                for (int col = 0; col < Cols; ++col)
                {
                    if (
                        grid[row][col].active &&
                        Distance(projectilePosition, CellPosition(row, col)) <= OrbRadius * 1.93f
                    )
                    {
                        collided = true;
                        break;
                    }
                }
            }
        }

        if (collided)
        {
            AttachProjectile();
        }
    }

    void AttachProjectile()
    {
        int bestRow = -1;
        int bestCol = -1;
        float bestDistance = 1000000.0f;

        for (int row = 0; row < Rows; ++row)
        {
            const int cellsInRow = row % 2 == 0 ? Cols : Cols - 1;
            for (int col = 0; col < cellsInRow; ++col)
            {
                if (grid[row][col].active)
                {
                    continue;
                }

                const float distance = Distance(projectilePosition, CellPosition(row, col));
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestRow = row;
                    bestCol = col;
                }
            }
        }

        projectileActive = false;

        if (bestRow < 0)
        {
            FailLevel();
            return;
        }

        int placedColor = currentColor;
        if (projectileIsPrism)
        {
            placedColor = FindNeighborColor(bestRow, bestCol);
        }

        grid[bestRow][bestCol].active = true;
        grid[bestRow][bestCol].color = placedColor;
        grid[bestRow][bestCol].armor = 0;
        grid[bestRow][bestCol].corrupted = false;

        const std::vector<std::pair<int, int>> cluster =
            FindColorCluster(bestRow, bestCol, placedColor);

        if (cluster.size() >= 3)
        {
            ResolveMatch(cluster);
        }
        else
        {
            combo = 0;
            --missesUntilPressure;
            message = Message::Miss;
            messageTimer = 1.0f;
            TriggerHitFeedback(
                projectilePosition,
                Color{ 255, 92, 76, 255 },
                false,
                0.42f
            );
            PlayFeedbackSound(missSound);

            if (missesUntilPressure <= 0)
            {
                AddPressureRow();
                missesUntilPressure = GetMissLimit();
            }
        }

        currentColor = nextColor;
        nextColor = PickAvailableColor();
        projectileIsPrism = false;

        if (HasReachedDangerLine())
        {
            TakeCorruptionHit();
        }
    }

    int FindNeighborColor(int row, int col) const
    {
        const auto neighbors = GetNeighbors(row, col);
        for (const auto& neighbor : neighbors)
        {
            const Cell& cell = grid[neighbor.first][neighbor.second];
            if (cell.active)
            {
                return cell.color;
            }
        }
        return currentColor;
    }

    std::vector<std::pair<int, int>> GetNeighbors(int row, int col) const
    {
        std::vector<std::pair<int, int>> result;
        const int offsetsEven[6][2] = {
            { 0, -1 }, { 0, 1 }, { -1, -1 }, { -1, 0 }, { 1, -1 }, { 1, 0 }
        };
        const int offsetsOdd[6][2] = {
            { 0, -1 }, { 0, 1 }, { -1, 0 }, { -1, 1 }, { 1, 0 }, { 1, 1 }
        };
        const int (*offsets)[2] = row % 2 == 0 ? offsetsEven : offsetsOdd;

        for (int i = 0; i < 6; ++i)
        {
            const int newRow = row + offsets[i][0];
            const int newCol = col + offsets[i][1];
            if (newRow >= 0 && newRow < Rows && newCol >= 0 && newCol < Cols)
            {
                result.emplace_back(newRow, newCol);
            }
        }
        return result;
    }

    std::vector<std::pair<int, int>> FindColorCluster(int startRow, int startCol, int color) const
    {
        std::array<std::array<bool, Cols>, Rows> visited{};
        std::queue<std::pair<int, int>> open;
        std::vector<std::pair<int, int>> cluster;
        open.emplace(startRow, startCol);
        visited[startRow][startCol] = true;

        while (!open.empty())
        {
            const auto current = open.front();
            open.pop();
            cluster.push_back(current);

            for (const auto& neighbor : GetNeighbors(current.first, current.second))
            {
                if (
                    !visited[neighbor.first][neighbor.second] &&
                    grid[neighbor.first][neighbor.second].active &&
                    !grid[neighbor.first][neighbor.second].corrupted &&
                    grid[neighbor.first][neighbor.second].color == color
                )
                {
                    visited[neighbor.first][neighbor.second] = true;
                    open.push(neighbor);
                }
            }
        }
        return cluster;
    }

    void ResolveMatch(const std::vector<std::pair<int, int>>& cluster)
    {
        ++combo;
        ++power;
        const int multiplier = std::min(4, combo);
        int shattered = 0;
        int armorCracked = 0;
        int corruptionDestroyed = 0;
        std::array<std::array<bool, Cols>, Rows> blastVisited{};

        for (const auto& item : cluster)
        {
            SpawnBurst(CellPosition(item.first, item.second), grid[item.first][item.second].color);
            if (grid[item.first][item.second].armor > 0)
            {
                grid[item.first][item.second].armor = 0;
                ++armorCracked;
            }
            else
            {
                grid[item.first][item.second].active = false;
                ++shattered;
            }

            for (const auto& neighbor : GetNeighbors(item.first, item.second))
            {
                Cell& nearby = grid[neighbor.first][neighbor.second];
                if (!nearby.active || blastVisited[neighbor.first][neighbor.second]) continue;
                if (nearby.corrupted)
                {
                    blastVisited[neighbor.first][neighbor.second] = true;
                    SpawnBurst(CellPosition(neighbor.first, neighbor.second), 1);
                    nearby = Cell{};
                    ++corruptionDestroyed;
                }
                else if (nearby.armor > 0)
                {
                    blastVisited[neighbor.first][neighbor.second] = true;
                    nearby.armor = 0;
                    ++armorCracked;
                }
            }
        }

        const int dropped = DropDisconnectedBubbles();
        poppedCount += shattered + dropped + corruptionDestroyed;
        score += shattered * 115 * multiplier;
        score += dropped * 175 * multiplier;
        score += armorCracked * 85;
        score += corruptionDestroyed * 150;
        missesUntilPressure = MissesBeforePressure;
        message = dropped >= 3 ? Message::BigDrop : Message::Match;
        messageTimer = 1.35f;
        shakeTimer = dropped >= 3 ? 0.32f : 0.22f;
        TriggerHitFeedback(
            projectilePosition,
            dropped >= 3 ? Color{ 255, 205, 74, 255 } : Color{ 62, 244, 159, 255 },
            true,
            dropped >= 3 ? 0.62f : 0.42f
        );
        PlayFeedbackSound(matchSound);

        if (power >= 3)
        {
            message = Message::PrismReady;
            messageTimer = 2.0f;
        }

        if (score >= GetPhaseTarget())
        {
            ++sealsBroken;
            message = Message::SealBroken;
            messageTimer = 2.4f;
            screenFlash = 0.72f;
            shakeTimer = 0.48f;
            TriggerHitFeedback(
                Vector2{ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.32f },
                GOLD,
                true,
                0.88f
            );
            PlayFeedbackSound(sealSound);

            if (sealsBroken >= SealCount)
            {
                portalOpen = true;
                projectileActive = false;
                score = GetPhaseTarget();
            }
            else
            {
                score = 0;
                combo = 0;
                power = 0;
                missesUntilPressure = GetMissLimit();
                phaseTimer = 0.0f;
                pressureTimer = GetPressureTime();
                coreShiftTimer = PhaseThreeCoreShiftTime;
                BuildOpeningBoard();
                currentColor = PickAvailableColor();
                nextColor = PickAvailableColor();
            }
        }
    }

    int DropDisconnectedBubbles()
    {
        std::array<std::array<bool, Cols>, Rows> connected{};
        std::queue<std::pair<int, int>> open;

        for (int col = 0; col < Cols; ++col)
        {
            if (grid[0][col].active)
            {
                connected[0][col] = true;
                open.emplace(0, col);
            }
        }

        while (!open.empty())
        {
            const auto current = open.front();
            open.pop();
            for (const auto& neighbor : GetNeighbors(current.first, current.second))
            {
                if (
                    grid[neighbor.first][neighbor.second].active &&
                    !connected[neighbor.first][neighbor.second]
                )
                {
                    connected[neighbor.first][neighbor.second] = true;
                    open.push(neighbor);
                }
            }
        }

        int dropped = 0;
        for (int row = 0; row < Rows; ++row)
        {
            for (int col = 0; col < Cols; ++col)
            {
                if (grid[row][col].active && !connected[row][col])
                {
                    SpawnBurst(CellPosition(row, col), grid[row][col].color);
                    grid[row][col].active = false;
                    ++dropped;
                }
            }
        }
        return dropped;
    }

    void AddPressureRow()
    {
        for (int row = Rows - 1; row > 0; --row)
        {
            grid[row] = grid[row - 1];
        }

        for (int col = 0; col < Cols; ++col)
        {
            grid[0][col].active = true;
            grid[0][col].color = GetRandomValue(0, ColorCount - 1);
            grid[0][col].armor = sealsBroken == 1 && col % 5 == 0 ? 1 : 0;
            grid[0][col].corrupted = sealsBroken == 2 && col % 6 == 0;
        }

        message = Message::Pressure;
        messageTimer = 1.8f;
        screenFlash = 0.38f;
        shakeTimer = 0.35f;
        TriggerHitFeedback(
            Vector2{ GetScreenWidth() * 0.5f, GetBoardRectangle().y + 70.0f },
            Color{ 255, 54, 102, 255 },
            false,
            0.55f
        );
        PlayFeedbackSound(missSound);
    }

    bool HasReachedDangerLine() const
    {
        for (int row = Rows - 2; row < Rows; ++row)
        {
            for (const Cell& cell : grid[row])
            {
                if (cell.active)
                {
                    return true;
                }
            }
        }
        return false;
    }

    void FailLevel()
    {
        failed = true;
        projectileActive = false;
        message = Message::None;
    }

    void TakeCorruptionHit()
    {
        --playerHealth;
        projectileActive = false;
        combo = 0;
        power = 0;
        score = 0;
        message = Message::PlayerHit;
        messageTimer = 2.2f;
        screenFlash = 0.72f;
        shakeTimer = 0.55f;

        if (playerHealth <= 0)
        {
            FailLevel();
            return;
        }

        missesUntilPressure = GetMissLimit();
        phaseTimer = 0.0f;
        pressureTimer = GetPressureTime();
        coreShiftTimer = PhaseThreeCoreShiftTime;
        BuildOpeningBoard();
        currentColor = PickAvailableColor();
        nextColor = PickAvailableColor();
    }

    static Sound CreateTone(float frequency, float duration, float volume)
    {
        constexpr unsigned int sampleRate = 22050;
        const unsigned int frameCount = static_cast<unsigned int>(sampleRate * duration);
        std::vector<short> samples(frameCount);

        for (unsigned int frame = 0; frame < frameCount; ++frame)
        {
            const float time = static_cast<float>(frame) / static_cast<float>(sampleRate);
            const float fade = 1.0f - static_cast<float>(frame) / static_cast<float>(frameCount);
            const float overtone = std::sin(time * frequency * 4.0f * PI) * 0.24f;
            const float signal = (std::sin(time * frequency * 2.0f * PI) + overtone) * fade;
            samples[frame] = static_cast<short>(signal * 32767.0f * volume);
        }

        Wave wave{};
        wave.frameCount = frameCount;
        wave.sampleRate = sampleRate;
        wave.sampleSize = 16;
        wave.channels = 1;
        wave.data = samples.data();
        return LoadSoundFromWave(wave);
    }

    void InitializeFeedbackAudio()
    {
        if (!IsAudioDeviceReady())
        {
            InitAudioDevice();
            ownsAudioDevice = IsAudioDeviceReady();
        }

        if (!IsAudioDeviceReady())
        {
            return;
        }

        matchSound = CreateTone(660.0f, 0.11f, 0.22f);
        missSound = CreateTone(145.0f, 0.14f, 0.24f);
        sealSound = CreateTone(880.0f, 0.30f, 0.25f);
        feedbackAudioReady = true;
    }

    void ShutdownFeedbackAudio()
    {
        if (feedbackAudioReady && IsAudioDeviceReady())
        {
            UnloadSound(matchSound);
            UnloadSound(missSound);
            UnloadSound(sealSound);
        }

        feedbackAudioReady = false;

        if (ownsAudioDevice && IsAudioDeviceReady())
        {
            CloseAudioDevice();
        }
    }

    void PlayFeedbackSound(Sound sound) const
    {
        if (feedbackAudioReady && IsAudioDeviceReady())
        {
            PlaySound(sound);
        }
    }

    void TriggerHitFeedback(
        Vector2 position,
        Color color,
        bool success,
        float duration
    )
    {
        impactPosition = position;
        hitFeedbackColor = color;
        hitFeedbackSuccess = success;
        hitFeedbackTimer = duration;
        screenFlash = std::max(screenFlash, duration * 0.72f);
    }

    void SpawnBurst(Vector2 position, int colorIndex)
    {
        for (int i = 0; i < 10; ++i)
        {
            const float angle = static_cast<float>(GetRandomValue(0, 628)) / 100.0f;
            const float speed = static_cast<float>(GetRandomValue(80, 250));
            Particle particle;
            particle.position = position;
            particle.velocity = Vector2{ std::cos(angle) * speed, std::sin(angle) * speed };
            particle.color = BubbleColor(colorIndex);
            particle.life = static_cast<float>(GetRandomValue(45, 90)) / 100.0f;
            particle.radius = static_cast<float>(GetRandomValue(3, 8));
            particles.push_back(particle);
        }
    }

    void UpdateParticles(float deltaTime)
    {
        for (Particle& particle : particles)
        {
            particle.life -= deltaTime;
            particle.position.x += particle.velocity.x * deltaTime;
            particle.position.y += particle.velocity.y * deltaTime;
            particle.velocity.y += 170.0f * deltaTime;
        }
        particles.erase(
            std::remove_if(
                particles.begin(),
                particles.end(),
                [](const Particle& particle) { return particle.life <= 0.0f; }
            ),
            particles.end()
        );
    }

    void DrawBubble(Vector2 position, int colorIndex, float radius, bool prism = false) const
    {
        const Color color = prism ? Color{ 242, 224, 156, 255 } : BubbleColor(colorIndex);
        const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetTime()) * 4.0f + position.x * 0.01f);
        DrawCircleV(position, radius + 8.0f, Fade(color, 0.12f + pulse * 0.10f));
        DrawPoly(position, 6, radius, 30.0f, Color{ static_cast<unsigned char>(color.r / 3), static_cast<unsigned char>(color.g / 3), static_cast<unsigned char>(color.b / 3), 255 });
        DrawPoly(position, 6, radius - 4.0f, 30.0f, color);
        DrawPolyLinesEx(position, 6, radius, 30.0f, 2.0f, Fade(RAYWHITE, 0.76f));
        DrawTriangle(
            Vector2{ position.x, position.y - radius + 5.0f },
            Vector2{ position.x - radius * 0.56f, position.y + radius * 0.40f },
            Vector2{ position.x + 2.0f, position.y + radius * 0.12f },
            Fade(RAYWHITE, 0.24f)
        );
        if (prism)
        {
            DrawPolyLinesEx(position, 6, radius - 7.0f, 30.0f, 2.0f, GOLD);
        }
    }

    void DrawCrystalCell(Vector2 position, const Cell& cell) const
    {
        if (cell.corrupted)
        {
            const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetTime()) * 6.0f + position.x);
            DrawCircleV(position, OrbRadius + 9.0f, Fade(Color{ 218, 35, 92, 255 }, 0.13f + pulse * 0.12f));
            DrawPoly(position, 8, OrbRadius - 1.0f, 22.5f, Color{ 46, 8, 28, 255 });
            DrawPolyLinesEx(position, 8, OrbRadius - 1.0f, 22.5f, 3.0f, Color{ 244, 58, 118, 255 });
            DrawLineEx(Vector2{ position.x - 10.0f, position.y - 11.0f }, Vector2{ position.x + 10.0f, position.y + 11.0f }, 4.0f, Color{ 255, 91, 132, 255 });
            DrawLineEx(Vector2{ position.x + 10.0f, position.y - 11.0f }, Vector2{ position.x - 10.0f, position.y + 11.0f }, 4.0f, Color{ 255, 91, 132, 255 });
            return;
        }

        DrawBubble(position, cell.color, OrbRadius - 2.0f);
        if (cell.armor > 0)
        {
            DrawRing(position, OrbRadius - 5.0f, OrbRadius + 5.0f, 0.0f, 360.0f, 24, Color{ 170, 185, 205, 220 });
            DrawLineEx(Vector2{ position.x - 15.0f, position.y }, Vector2{ position.x + 15.0f, position.y }, 3.0f, RAYWHITE);
        }
    }

    void DrawGrid() const
    {
        Vector2 offset{};
        if (shakeTimer > 0.0f)
        {
            offset = Vector2{
                static_cast<float>(GetRandomValue(-4, 4)),
                static_cast<float>(GetRandomValue(-3, 3))
            };
        }

        for (int row = 0; row < Rows; ++row)
        {
            for (int col = 0; col < Cols; ++col)
            {
                if (grid[row][col].active)
                {
                    Vector2 position = CellPosition(row, col);
                    position.x += offset.x;
                    position.y += offset.y;
                    DrawCrystalCell(position, grid[row][col]);
                }
            }
        }

        if (projectileActive)
        {
            const Vector2 trailDirection = Normalize(projectileVelocity);
            DrawLineEx(
                Vector2{ projectilePosition.x - trailDirection.x * 35.0f, projectilePosition.y - trailDirection.y * 35.0f },
                projectilePosition,
                8.0f,
                Fade(projectileIsPrism ? GOLD : BubbleColor(currentColor), 0.32f)
            );
            DrawBubble(projectilePosition, currentColor, OrbRadius - 2.0f, projectileIsPrism);
        }
    }

    void DrawAimGuide() const
    {
        if (projectileActive)
        {
            return;
        }

        const Rectangle board = GetBoardRectangle();
        const Vector2 launcher = GetLauncherPosition();
        Vector2 aim = GetMousePosition();
        aim.y = std::min(aim.y, launcher.y - 80.0f);
        Vector2 direction = Normalize(Vector2{ aim.x - launcher.x, aim.y - launcher.y });
        if (direction.y > -0.18f)
        {
            direction.y = -0.18f;
            direction = Normalize(direction);
        }

        Vector2 point = launcher;
        Vector2 velocity = Vector2{ direction.x * 24.0f, direction.y * 24.0f };
        for (int i = 0; i < 22; ++i)
        {
            point.x += velocity.x;
            point.y += velocity.y;
            if (point.x < board.x + OrbRadius || point.x > board.x + board.width - OrbRadius)
            {
                velocity.x *= -1.0f;
                point.x = std::clamp(point.x, board.x + OrbRadius, board.x + board.width - OrbRadius);
            }
            DrawCircleV(point, 3.2f, Fade(RAYWHITE, 0.52f));
            if (point.y < board.y + 30.0f)
            {
                break;
            }
        }
    }

    void DrawAetherWarden() const
    {
        const Vector2 launcher = GetLauncherPosition();
        const float walkScale = portalOpen ? 1.0f - portalWalk * 0.45f : 1.0f;
        const float walkLift = portalOpen ? portalWalk * 145.0f : 0.0f;
        const Vector2 body{ launcher.x, launcher.y + 50.0f - walkLift };
        Vector2 aim = GetMousePosition();
        aim.y = std::min(aim.y, launcher.y - 80.0f);
        Vector2 direction = Normalize(Vector2{ aim.x - launcher.x, aim.y - launcher.y });
        if (direction.y > -0.18f)
        {
            direction.y = -0.18f;
            direction = Normalize(direction);
        }

        (void)walkScale;
        DrawEllipse(static_cast<int>(body.x), static_cast<int>(body.y + 37.0f), 34.0f, 10.0f, Fade(BLACK, 0.45f * (1.0f - portalWalk * 0.7f)));
        DrawLineEx(Vector2{ body.x - 11.0f, body.y + 20.0f }, Vector2{ body.x - 19.0f, body.y + 38.0f }, 9.0f, Color{ 22, 29, 43, 255 });
        DrawLineEx(Vector2{ body.x + 11.0f, body.y + 20.0f }, Vector2{ body.x + 19.0f, body.y + 38.0f }, 9.0f, Color{ 22, 29, 43, 255 });
        DrawRectangleRounded(Rectangle{ body.x - 22.0f, body.y - 18.0f, 44.0f, 48.0f }, 0.25f, 8, Color{ 35, 45, 65, 255 });
        DrawRectangleRoundedLinesEx(Rectangle{ body.x - 22.0f, body.y - 18.0f, 44.0f, 48.0f }, 0.25f, 8, 2.0f, Color{ 106, 142, 184, 255 });
        DrawCircleV(Vector2{ body.x, body.y - 32.0f }, 17.0f, Color{ 31, 39, 57, 255 });
        DrawTriangle(Vector2{ body.x - 17.0f, body.y - 34.0f }, Vector2{ body.x, body.y - 58.0f }, Vector2{ body.x + 17.0f, body.y - 34.0f }, Color{ 49, 62, 88, 255 });
        DrawLineEx(Vector2{ body.x - 12.0f, body.y - 4.0f }, Vector2{ body.x - 30.0f, body.y + 13.0f }, 10.0f, Color{ 41, 52, 73, 255 });

        const Vector2 swordGrip{
            launcher.x - direction.x * 74.0f,
            launcher.y - direction.y * 74.0f
        };
        const Vector2 swordTip{ launcher.x, launcher.y };
        DrawLineEx(Vector2{ body.x + 12.0f, body.y - 4.0f }, swordGrip, 10.0f, Color{ 41, 52, 73, 255 });
        DrawCircleV(swordGrip, 7.0f, Color{ 125, 91, 51, 255 });
        DrawLineEx(swordGrip, swordTip, 8.0f, Color{ 117, 86, 48, 255 });
        DrawLineEx(
            Vector2{ swordGrip.x + direction.x * 8.0f, swordGrip.y + direction.y * 8.0f },
            swordTip,
            4.0f,
            Color{ 184, 228, 245, 255 }
        );
        if (!portalOpen)
        {
            DrawCircleV(swordTip, 34.0f, Fade(BubbleColor(currentColor), 0.12f));
            DrawCircleLinesV(swordTip, 31.0f, Fade(BubbleColor(currentColor), 0.62f));
            DrawBubble(launcher, currentColor, OrbRadius, power >= 3);
        }

        if (!portalOpen)
        {
            const Vector2 nextPosition{ launcher.x + 86.0f, launcher.y + 5.0f };
            DrawText("RESERVE [Q/RMB]", static_cast<int>(nextPosition.x - 57.0f), static_cast<int>(nextPosition.y - 44.0f), 13, LIGHTGRAY);
            DrawBubble(nextPosition, nextColor, 15.0f);
        }
    }

    void DrawHUD() const
    {
        const int width = GetScreenWidth();
        const char* title = "LEVEL 2  -  THE SHATTERED SEAL  [V11 - TITAN REWORK]";
        DrawText(title, width / 2 - MeasureText(title, 28) / 2, 26, 28, RAYWHITE);
        DrawText(
            portalOpen ? "LEVEL 2 COMPLETE  -  ENTERING LEVEL 3" : "A/D MOVE  |  LEFT CLICK FIRE  |  Q OR RIGHT CLICK SWAP CORES",
            width / 2 - MeasureText(portalOpen ? "LEVEL 2 COMPLETE  -  ENTERING LEVEL 3" : "A/D MOVE  |  LEFT CLICK FIRE  |  Q OR RIGHT CLICK SWAP CORES", 17) / 2,
            65,
            17,
            Color{ 107, 225, 255, 255 }
        );

        DrawRectangleRounded(Rectangle{ 22.0f, 112.0f, 205.0f, 372.0f }, 0.08f, 8, Color{ 4, 15, 27, 225 });
        DrawRectangleRoundedLinesEx(Rectangle{ 22.0f, 112.0f, 205.0f, 372.0f }, 0.08f, 8, 2.0f, Color{ 45, 140, 180, 220 });
        DrawText("LEVEL 1 AETHER CORE", 32, 133, 17, Color{ 116, 233, 255, 255 });
        DrawCircleV(Vector2{ 124.0f, 166.0f }, 16.0f, Fade(Color{ 91, 220, 255, 255 }, 0.35f));
        DrawPoly(Vector2{ 124.0f, 166.0f }, 6, 11.0f, 30.0f, Color{ 189, 244, 255, 255 });
        for (int seal = 0; seal < SealCount; ++seal)
        {
            const Vector2 center{ 67.0f + seal * 57.0f, 216.0f };
            const bool broken = seal < sealsBroken;
            DrawPoly(center, 6, 20.0f, 30.0f, broken ? Color{ 42, 255, 164, 255 } : Color{ 37, 49, 66, 255 });
            DrawPolyLinesEx(center, 6, 20.0f, 30.0f, 2.0f, broken ? RAYWHITE : Color{ 90, 112, 137, 255 });
            if (broken)
            {
                DrawLineEx(Vector2{ center.x - 9.0f, center.y - 11.0f }, Vector2{ center.x + 8.0f, center.y + 12.0f }, 3.0f, Color{ 7, 28, 28, 255 });
            }
        }
        DrawText(TextFormat("SEALS  %d / %d", sealsBroken, SealCount), 43, 250, 21, sealsBroken == SealCount ? LIME : GOLD);
        DrawText(TextFormat("WARDEN HP  %d / %d", playerHealth, MaxPlayerHealth), 43, 280, 18, playerHealth == 1 ? RED : RAYWHITE);
        DrawText(TextFormat("SHARDS %d", poppedCount), 43, 308, 18, Color{ 120, 240, 170, 255 });
        DrawText(TextFormat("RESONANCE x%d", std::max(1, combo)), 43, 336, 16, Color{ 220, 150, 255, 255 });

        const char* mechanic =
            sealsBroken == 0
            ? "I  RESONANCE: FORGIVING MATCHES"
            : sealsBroken == 1
            ? "II  ARMOR: BREAK SILVER SHELLS"
            : "III  CORRUPTION: SHIFTING CORES";
        DrawText(mechanic, 27, 366, 14, sealsBroken == 2 ? Color{ 255, 108, 155, 255 } : Color{ 137, 220, 255, 255 });

        DrawText("PRISM CHARGE", 34, 398, 16, LIGHTGRAY);
        for (int i = 0; i < 3; ++i)
        {
            const Color barColor = i < power ? Color{ 204, 103, 255, 255 } : Color{ 32, 53, 70, 255 };
            DrawRectangle(35 + i * 58, 426, 48, 14, barColor);
        }
        DrawText("3 COMBOS = PRISM CORE", 27, 454, 14, Color{ 220, 190, 130, 255 });

        const float progress = portalOpen ? 1.0f : std::clamp(static_cast<float>(score) / GetPhaseTarget(), 0.0f, 1.0f);
        const Rectangle progressBack{ width / 2.0f - 245.0f, GetScreenHeight() - 46.0f, 490.0f, 17.0f };
        DrawRectangleRounded(progressBack, 0.5f, 8, Color{ 20, 37, 52, 255 });
        DrawRectangleRounded(
            Rectangle{ progressBack.x, progressBack.y, progressBack.width * progress, progressBack.height },
            0.5f,
            8,
            Color{ 55, 225, 144, 255 }
        );
        // The Warden stands at the lower centre of the screen. Keeping phase
        // text here used to cover the character, so the progress bar now
        // communicates the same information without another centred label.

        if (!portalOpen && sealsBroken == 1)
        {
            DrawText("ARMORED CRYSTALS: DETONATE A MATCH BESIDE THEIR SILVER SHELL", 24, 482, 14, Color{ 195, 211, 231, 255 });
        }
        else if (!portalOpen && sealsBroken == 2)
        {
            DrawText(TextFormat("SURGE %.1f  |  CORE SHIFT %.1f", std::max(0.0f, pressureTimer), std::max(0.0f, coreShiftTimer)), 24, 482, 16, Color{ 255, 102, 132, 255 });
        }

        if (messageTimer > 0.0f)
        {
            const char* text = "";
            Color color = RAYWHITE;
            switch (message)
            {
            case Message::Intro: text = "CHANNEL THE BLADE. SHATTER MATCHING AETHER CRYSTALS."; color = Color{ 115, 230, 255, 255 }; break;
            case Message::Match: text = combo > 1 ? "AETHER RESONANCE!" : "CRYSTALS SHATTERED!"; color = LIME; break;
            case Message::BigDrop: text = "AETHER CASCADE - MASSIVE SEAL DAMAGE!"; color = GOLD; break;
            case Message::Miss: text = "NO RESONANCE - CORRUPTION RISES"; color = ORANGE; break;
            case Message::Pressure: text = "THE CORRUPTION DESCENDS!"; color = RED; break;
            case Message::PrismReady: text = "PRISM CORE FORGED - RESONATES WITH EVERY AETHER"; color = Color{ 242, 220, 145, 255 }; break;
            case Message::SealBroken: text = TextFormat("ANCIENT SEAL %d SHATTERED!", sealsBroken); color = GOLD; break;
            case Message::CoreSwapped: text = "AETHER CORES SWAPPED"; color = Color{ 140, 225, 255, 255 }; break;
            case Message::PlayerHit: text = "CORRUPTION STRUCK THE WARDEN - CURRENT SEAL RESTARTED"; color = RED; break;
            case Message::CorruptionShift: text = "CORRUPTION REFORGED THE ACTIVE AETHER CORES!"; color = Color{ 222, 112, 255, 255 }; break;
            default: break;
            }
            DrawText(text, width / 2 - MeasureText(text, 20) / 2, 91, 20, color);
        }
    }

    void DrawPortal() const
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        DrawRectangle(0, 95, width, height - 190, Color{ 0, 8, 15, 125 });

        const Rectangle portal = GetPortalRectangle();
        const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetTime()) * 4.0f);
        DrawCircleV(Vector2{ width / 2.0f, portal.y + portal.height / 2.0f }, 118.0f, Fade(Color{ 80, 210, 255, 255 }, 0.13f + pulse * 0.12f));
        DrawRing(Vector2{ width / 2.0f, portal.y + portal.height / 2.0f }, 72.0f, 104.0f, 0.0f, 360.0f, 64, Color{ 29, 39, 61, 255 });
        DrawRing(Vector2{ width / 2.0f, portal.y + portal.height / 2.0f }, 84.0f, 94.0f, 0.0f, 360.0f, 64, Color{ 92, 226, 255, 255 });
        DrawCircleV(Vector2{ width / 2.0f, portal.y + portal.height / 2.0f }, 75.0f, Color{ 29, 96, 121, 255 });
        for (int ring = 0; ring < 4; ++ring)
        {
            DrawCircleLinesV(Vector2{ width / 2.0f, portal.y + portal.height / 2.0f }, 56.0f - ring * 11.0f + pulse * 3.0f, Fade(RAYWHITE, 0.22f));
        }
        DrawText("LEVEL 2 COMPLETE", width / 2 - MeasureText("LEVEL 2 COMPLETE", 38) / 2, static_cast<int>(portal.y - 92.0f), 38, LIME);
        DrawText("THE THREE SEALS ARE BROKEN", width / 2 - MeasureText("THE THREE SEALS ARE BROKEN", 24) / 2, static_cast<int>(portal.y - 48.0f), 24, GOLD);
        DrawText("LEVEL 3  -  AETHER TITAN", width / 2 - MeasureText("LEVEL 3  -  AETHER TITAN", 25) / 2, static_cast<int>(portal.y + 44.0f), 25, RAYWHITE);
        DrawText("THE WARDEN IS ENTERING THE GATE...", width / 2 - MeasureText("THE WARDEN IS ENTERING THE GATE...", 18) / 2, static_cast<int>(portal.y + 92.0f), 18, Color{ 150, 235, 255, 255 });
        DrawRectangleRounded(Rectangle{ width / 2.0f - 120.0f, portal.y + 122.0f, 240.0f, 12.0f }, 0.5f, 8, Color{ 19, 42, 54, 255 });
        DrawRectangleRounded(Rectangle{ width / 2.0f - 120.0f, portal.y + 122.0f, 240.0f * portalWalk, 12.0f }, 0.5f, 8, Color{ 87, 230, 255, 255 });
    }

    void DrawFailurePanel() const
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        DrawRectangle(0, 0, width, height, Color{ 0, 0, 0, 190 });
        DrawText("CORRUPTION CONSUMED THE SANCTUM", width / 2 - MeasureText("CORRUPTION CONSUMED THE SANCTUM", 38) / 2, height / 2 - 85, 38, RED);
        DrawText(TextFormat("SEALS BROKEN  %d / %d", sealsBroken, SealCount), width / 2 - MeasureText(TextFormat("SEALS BROKEN  %d / %d", sealsBroken, SealCount), 25) / 2, height / 2 - 18, 25, RAYWHITE);
        DrawText("PRESS R OR ENTER TO RESTORE THE LEVEL 1 AETHER CORE", width / 2 - MeasureText("PRESS R OR ENTER TO RESTORE THE LEVEL 1 AETHER CORE", 22) / 2, height / 2 + 42, 22, Color{ 112, 226, 255, 255 });
    }

    void DrawIntroPanel() const
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        const float alpha = std::clamp(introTimer, 0.0f, 1.0f);
        DrawRectangle(0, 0, width, height, Fade(Color{ 0, 4, 12, 255 }, 0.66f * alpha));
        const char* heading = "THE AETHER CORE REACTS";
        const char* line1 = "THE CORE CLAIMED IN LEVEL 1 HAS AWAKENED THE SHATTERED GATE.";
        const char* line2 = "THE WARDEN MUST CHANNEL IT THROUGH THE SAME BLADE THAT WON IT.";
        DrawText(heading, width / 2 - MeasureText(heading, 34) / 2, height / 2 - 82, 34, GOLD);
        DrawText(line1, width / 2 - MeasureText(line1, 18) / 2, height / 2 - 23, 18, RAYWHITE);
        DrawText(line2, width / 2 - MeasureText(line2, 18) / 2, height / 2 + 10, 18, Color{ 126, 226, 255, 255 });
        DrawText("A/D MOVE   |   LEFT CLICK FIRE   |   Q / RIGHT CLICK SWAP", width / 2 - MeasureText("A/D MOVE   |   LEFT CLICK FIRE   |   Q / RIGHT CLICK SWAP", 17) / 2, height / 2 + 61, 17, LIGHTGRAY);
    }

    void DrawParticles() const
    {
        for (const Particle& particle : particles)
        {
            DrawCircleV(particle.position, particle.radius, Fade(particle.color, std::clamp(particle.life, 0.0f, 1.0f)));
        }
    }

    Vector3 BoardToWorld(Vector2 point, float depth = -8.35f) const
    {
        const Ray ray = GetScreenToWorldRay(point, challengeCamera);

        if (std::fabs(ray.direction.z) < 0.0001f)
        {
            return Vector3{ 0.0f, 3.0f, depth };
        }

        const float distance =
            (depth - ray.position.z) /
            ray.direction.z;

        return Vector3Add(
            ray.position,
            Vector3Scale(ray.direction, distance)
        );
    }

    Vector3 ProjectileWorldPosition() const
    {
        const Rectangle board = GetBoardRectangle();
        const Vector2 launcher = GetLauncherPosition();
        const float travel = std::clamp(
            (launcher.y - projectilePosition.y) /
            std::max(1.0f, launcher.y - board.y - 30.0f),
            0.0f,
            1.0f
        );

        const float depth = 3.9f - 12.25f * travel;
        return BoardToWorld(projectilePosition, depth);
    }

    void DrawGateArchitecture() const
    {
        const Color stone{ 31, 39, 54, 255 };
        const Color edge{ 79, 106, 131, 255 };

        DrawCube(Vector3{ -9.4f, 3.8f, -9.0f }, 2.2f, 7.6f, 2.0f, stone);
        DrawCubeWires(Vector3{ -9.4f, 3.8f, -9.0f }, 2.2f, 7.6f, 2.0f, edge);
        DrawCube(Vector3{ 9.4f, 3.8f, -9.0f }, 2.2f, 7.6f, 2.0f, stone);
        DrawCubeWires(Vector3{ 9.4f, 3.8f, -9.0f }, 2.2f, 7.6f, 2.0f, edge);
        DrawCube(Vector3{ 0.0f, 8.0f, -9.0f }, 20.8f, 1.6f, 2.0f, stone);
        DrawCubeWires(Vector3{ 0.0f, 8.0f, -9.0f }, 20.8f, 1.6f, 2.0f, edge);

        for (int rune = -4; rune <= 4; ++rune)
        {
            const float pulse = 0.45f + 0.25f * std::sin(
                static_cast<float>(GetTime()) * 2.4f + rune
            );
            DrawCube(
                Vector3{ rune * 2.0f, 7.95f, -7.94f },
                0.55f,
                0.16f,
                0.12f,
                Fade(Color{ 80, 218, 255, 255 }, pulse)
            );
        }
    }

    void DrawSealMonoliths() const
    {
        static constexpr float sealX[SealCount] = { -6.4f, 0.0f, 6.4f };

        for (int index = 0; index < SealCount; ++index)
        {
            const bool broken = index < sealsBroken;
            const float height = broken ? 1.0f : 2.6f;
            const float y = 8.6f + height * 0.5f;
            const Color glow = broken
                ? Color{ 39, 54, 62, 255 }
                : index == sealsBroken
                    ? Color{ 88, 228, 255, 255 }
                    : Color{ 74, 93, 117, 255 };

            DrawCube(Vector3{ sealX[index], y, -9.0f }, 1.25f, height, 1.1f, Color{ 27, 32, 46, 255 });
            DrawCubeWires(Vector3{ sealX[index], y, -9.0f }, 1.25f, height, 1.1f, glow);

            if (!broken)
            {
                DrawSphere(Vector3{ sealX[index], y + height * 0.55f, -8.8f }, 0.35f, glow);
            }
        }
    }

    void DrawCrystalWall3D() const
    {
        DrawCube(Vector3{ 0.0f, 4.6f, -8.7f }, 17.0f, 7.0f, 0.45f, Color{ 8, 13, 25, 235 });

        for (int row = 0; row < Rows; ++row)
        {
            for (int col = 0; col < Cols; ++col)
            {
                const Cell& cell = grid[row][col];
                if (!cell.active)
                {
                    continue;
                }

                const Vector3 position = BoardToWorld(CellPosition(row, col));
                const Color color = cell.corrupted
                    ? Color{ 225, 43, 106, 255 }
                    : BubbleColor(cell.color);

                DrawSphereEx(position, 0.48f, 6, 6, color);
                DrawSphereWires(position, 0.50f, 6, 6, Fade(RAYWHITE, 0.65f));

                if (cell.armor > 0)
                {
                    DrawSphereWires(position, 0.66f, 8, 8, Color{ 195, 211, 229, 255 });
                }
            }
        }
    }

    void DrawProjectile3D(const Player& player) const
    {
        const Vector3 swordCore{
            player.GetPosition().x + 0.30f,
            player.GetPosition().y + 1.72f,
            player.GetPosition().z - 0.45f
        };
        const Color coreColor = projectileIsPrism ? GOLD : BubbleColor(currentColor);

        if (projectileActive)
        {
            const Vector3 projectile = ProjectileWorldPosition();
            DrawLine3D(swordCore, projectile, Fade(coreColor, 0.48f));
            DrawSphereEx(projectile, 0.36f, 8, 8, coreColor);
            DrawSphereWires(projectile, 0.43f, 8, 8, RAYWHITE);
        }
        else
        {
            DrawSphereEx(swordCore, 0.25f, 8, 8, power >= 3 ? GOLD : coreColor);
            DrawSphereWires(swordCore, 0.31f, 8, 8, RAYWHITE);
        }
    }

    void DrawPortal3D() const
    {
        const float pulse = 0.88f + 0.12f * std::sin(static_cast<float>(GetTime()) * 4.0f);
        const Vector3 center{ 0.0f, 3.8f, -8.45f };

        DrawSphereEx(center, 2.65f * pulse, 18, 18, Fade(Color{ 57, 196, 255, 255 }, 0.58f));
        DrawSphereWires(center, 2.85f, 18, 18, RAYWHITE);

        for (int ring = 0; ring < 4; ++ring)
        {
            DrawSphereWires(
                center,
                3.05f + ring * 0.18f,
                12,
                12,
                Fade(Color{ 116, 230, 255, 255 }, 0.45f - ring * 0.08f)
            );
        }
    }

    void DrawRuinedSanctum() const
    {
        const float time = static_cast<float>(GetTime());
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();

        DrawCircleV(Vector2{ width * 0.5f, height * 0.34f }, 260.0f, Fade(Color{ 40, 72, 112, 255 }, 0.16f));
        DrawRectangle(0, height - 95, width, 95, Color{ 9, 12, 20, 255 });
        DrawTriangle(Vector2{ 0.0f, static_cast<float>(height - 95) }, Vector2{ width * 0.24f, static_cast<float>(height - 205) }, Vector2{ width * 0.38f, static_cast<float>(height - 95) }, Color{ 13, 18, 29, 255 });
        DrawTriangle(Vector2{ width * 0.64f, static_cast<float>(height - 95) }, Vector2{ width * 0.83f, static_cast<float>(height - 225) }, Vector2{ static_cast<float>(width), static_cast<float>(height - 95) }, Color{ 13, 18, 29, 255 });

        for (int pillar = 0; pillar < 4; ++pillar)
        {
            const float x = pillar < 2 ? 25.0f + pillar * 105.0f : width - 235.0f + (pillar - 2) * 105.0f;
            const float top = 170.0f + (pillar % 2) * 75.0f;
            DrawRectangle(static_cast<int>(x), static_cast<int>(top), 48, height - static_cast<int>(top) - 90, Color{ 15, 21, 33, 255 });
            DrawRectangle(static_cast<int>(x - 9.0f), static_cast<int>(top - 14.0f), 66, 18, Color{ 22, 29, 44, 255 });
            DrawLineEx(Vector2{ x + 10.0f, top + 55.0f }, Vector2{ x + 34.0f, top + 105.0f }, 3.0f, Fade(Color{ 78, 191, 255, 255 }, 0.35f));
        }

        for (int stone = 0; stone < 8; ++stone)
        {
            const float x = std::fmod(stone * 211.0f + time * (3.0f + stone % 3), static_cast<float>(width));
            const float y = 120.0f + std::fmod(stone * 127.0f, std::max(140.0f, height - 300.0f));
            DrawPoly(Vector2{ x, y + std::sin(time + stone) * 5.0f }, 5, 10.0f + stone % 4 * 3.0f, 18.0f, Color{ 25, 34, 50, 230 });
        }

        for (int i = 0; i < 24; ++i)
        {
            const float x = std::fmod(i * 173.0f + time * (8.0f + i % 5), static_cast<float>(width));
            const float y = std::fmod(i * 97.0f + time * (13.0f + i % 4), static_cast<float>(height));
            DrawCircleV(Vector2{ x, y }, 2.0f + (i % 3), Fade(Color{ 80, 220, 255, 255 }, 0.18f));
        }
    }
};
