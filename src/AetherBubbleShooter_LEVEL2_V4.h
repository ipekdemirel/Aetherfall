#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <queue>
#include <vector>

#include <raylib.h>

class AetherBubbleShooter
{
public:
    AetherBubbleShooter()
    {
        Start();
    }

    void Start()
    {
        for (auto& row : grid)
        {
            for (Cell& cell : row)
            {
                cell.active = false;
                cell.color = 0;
            }
        }

        score = 0;
        shotsLeft = MaxShots;
        poppedCount = 0;
        combo = 0;
        missesUntilPressure = MissesBeforePressure;
        power = 0;
        projectileActive = false;
        portalOpen = false;
        completed = false;
        failed = false;
        message = Message::Intro;
        messageTimer = 4.0f;
        screenFlash = 0.0f;
        shakeTimer = 0.0f;
        particles.clear();

        BuildOpeningBoard();
        currentColor = PickAvailableColor();
        nextColor = PickAvailableColor();
    }

    void Update(float deltaTime)
    {
        UpdateParticles(deltaTime);
        messageTimer = std::max(0.0f, messageTimer - deltaTime);
        screenFlash = std::max(0.0f, screenFlash - deltaTime);
        shakeTimer = std::max(0.0f, shakeTimer - deltaTime);

        if (completed)
        {
            return;
        }

        if (portalOpen)
        {
            const Rectangle portal = GetPortalRectangle();
            if (
                IsKeyPressed(KEY_ENTER) ||
                (
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                    CheckCollisionPointRec(GetMousePosition(), portal)
                )
            )
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

        if (!projectileActive && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            FireProjectile();
        }

        if (projectileActive)
        {
            UpdateProjectile(deltaTime);
        }
    }

    void Draw() const
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();

        DrawRectangleGradientV(
            0,
            0,
            width,
            height,
            Color{ 4, 10, 24, 255 },
            Color{ 4, 35, 43, 255 }
        );

        DrawBackgroundEnergy();

        const Rectangle board = GetBoardRectangle();
        DrawRectangleRounded(board, 0.035f, 10, Color{ 5, 18, 34, 238 });
        DrawRectangleRoundedLinesEx(
            board,
            0.035f,
            10,
            3.0f,
            Color{ 49, 180, 220, 220 }
        );

        const float dangerY = CellPosition(Rows - 2, 0).y + OrbRadius;
        DrawLineEx(
            Vector2{ board.x + 12.0f, dangerY },
            Vector2{ board.x + board.width - 12.0f, dangerY },
            2.0f,
            Color{ 255, 80, 90, 150 }
        );
        DrawText(
            "DANGER",
            static_cast<int>(board.x + 15.0f),
            static_cast<int>(dangerY - 22.0f),
            14,
            Color{ 255, 105, 115, 210 }
        );

        DrawGrid();
        DrawParticles();

        if (!portalOpen && !failed)
        {
            DrawAimGuide();
            DrawLauncher();
        }

        DrawHUD();

        if (portalOpen)
        {
            DrawPortal();
        }

        if (failed)
        {
            DrawFailurePanel();
        }

        if (screenFlash > 0.0f)
        {
            DrawRectangle(
                0,
                0,
                width,
                height,
                Fade(RAYWHITE, screenFlash * 0.32f)
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
    static constexpr int MaxShots = 34;
    static constexpr int TargetScore = 4200;
    static constexpr int MissesBeforePressure = 5;
    static constexpr float OrbRadius = 24.0f;
    static constexpr float ProjectileSpeed = 760.0f;

    struct Cell
    {
        bool active = false;
        int color = 0;
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
        PrismReady
    };

    std::array<std::array<Cell, Cols>, Rows> grid{};
    std::vector<Particle> particles;

    int score = 0;
    int shotsLeft = MaxShots;
    int poppedCount = 0;
    int combo = 0;
    int missesUntilPressure = MissesBeforePressure;
    int power = 0;

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

    Rectangle GetBoardRectangle() const
    {
        const float boardWidth = Cols * OrbRadius * 2.0f + OrbRadius + 34.0f;
        const float boardHeight = 590.0f;
        return Rectangle{
            GetScreenWidth() / 2.0f - boardWidth / 2.0f,
            108.0f,
            boardWidth,
            std::min(boardHeight, GetScreenHeight() - 180.0f)
        };
    }

    Vector2 GetLauncherPosition() const
    {
        const Rectangle board = GetBoardRectangle();
        return Vector2{
            board.x + board.width / 2.0f,
            board.y + board.height - 34.0f
        };
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
            Color{ 255, 69, 105, 255 },
            Color{ 54, 174, 255, 255 },
            Color{ 255, 196, 48, 255 },
            Color{ 71, 224, 132, 255 },
            Color{ 185, 92, 255, 255 }
        };
        return colors[index % ColorCount];
    }

    void BuildOpeningBoard()
    {
        for (int row = 0; row < 5; ++row)
        {
            const int cellsInRow = row % 2 == 0 ? Cols : Cols - 1;
            for (int col = 0; col < cellsInRow; ++col)
            {
                grid[row][col].active = true;
                grid[row][col].color = (col / 2 + row * 2 + (col % 3 == 0 ? 1 : 0)) % ColorCount;
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
                if (cell.active && !available[cell.color])
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
        --shotsLeft;
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

            if (missesUntilPressure <= 0)
            {
                AddPressureRow();
                missesUntilPressure = MissesBeforePressure;
            }
        }

        currentColor = nextColor;
        nextColor = PickAvailableColor();
        projectileIsPrism = false;

        if (score >= TargetScore)
        {
            portalOpen = true;
            message = Message::None;
            screenFlash = 0.8f;
        }
        else if (shotsLeft <= 0)
        {
            FailLevel();
        }
        else if (HasReachedDangerLine())
        {
            FailLevel();
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

        for (const auto& item : cluster)
        {
            SpawnBurst(CellPosition(item.first, item.second), grid[item.first][item.second].color);
            grid[item.first][item.second].active = false;
        }

        const int dropped = DropDisconnectedBubbles();
        poppedCount += static_cast<int>(cluster.size()) + dropped;
        score += static_cast<int>(cluster.size()) * 120 * multiplier;
        score += dropped * 190 * multiplier;
        missesUntilPressure = MissesBeforePressure;
        message = dropped >= 3 ? Message::BigDrop : Message::Match;
        messageTimer = 1.35f;
        shakeTimer = 0.18f;

        if (power >= 3)
        {
            message = Message::PrismReady;
            messageTimer = 2.0f;
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
        }

        message = Message::Pressure;
        messageTimer = 1.8f;
        screenFlash = 0.38f;
        shakeTimer = 0.35f;
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
        const Color color = prism ? Color{ 215, 120, 255, 255 } : BubbleColor(colorIndex);
        DrawCircleV(position, radius + 6.0f, Fade(color, 0.20f));
        DrawCircleV(position, radius, color);
        DrawCircleLinesV(position, radius, Fade(RAYWHITE, 0.78f));
        DrawCircleV(
            Vector2{ position.x - radius * 0.30f, position.y - radius * 0.33f },
            radius * 0.25f,
            Fade(RAYWHITE, 0.70f)
        );
        if (prism)
        {
            DrawCircleLinesV(position, radius - 6.0f, GOLD);
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
                    DrawBubble(position, grid[row][col].color, OrbRadius - 2.0f);
                }
            }
        }

        if (projectileActive)
        {
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

    void DrawLauncher() const
    {
        const Vector2 launcher = GetLauncherPosition();
        DrawCircleV(launcher, 39.0f, Color{ 11, 33, 53, 255 });
        DrawCircleLinesV(launcher, 39.0f, Color{ 94, 225, 255, 255 });
        DrawBubble(launcher, currentColor, OrbRadius, power >= 3);

        const Vector2 nextPosition{ launcher.x + 86.0f, launcher.y + 5.0f };
        DrawText("NEXT", static_cast<int>(nextPosition.x - 19.0f), static_cast<int>(nextPosition.y - 44.0f), 14, LIGHTGRAY);
        DrawBubble(nextPosition, nextColor, 15.0f);
    }

    void DrawHUD() const
    {
        const int width = GetScreenWidth();
        const char* title = "LEVEL 2  -  AETHER BUBBLE STRIKE";
        DrawText(title, width / 2 - MeasureText(title, 28) / 2, 26, 28, RAYWHITE);
        DrawText(
            "AIM WITH MOUSE  |  LEFT CLICK TO SHOOT  |  MATCH 3+  |  WALL SHOTS BOUNCE",
            width / 2 - MeasureText("AIM WITH MOUSE  |  LEFT CLICK TO SHOOT  |  MATCH 3+  |  WALL SHOTS BOUNCE", 17) / 2,
            65,
            17,
            Color{ 107, 225, 255, 255 }
        );

        DrawRectangleRounded(Rectangle{ 22.0f, 112.0f, 205.0f, 245.0f }, 0.08f, 8, Color{ 4, 15, 27, 225 });
        DrawRectangleRoundedLinesEx(Rectangle{ 22.0f, 112.0f, 205.0f, 245.0f }, 0.08f, 8, 2.0f, Color{ 45, 140, 180, 220 });
        DrawText("VAULT STATUS", 43, 133, 21, Color{ 116, 233, 255, 255 });
        DrawText(TextFormat("SCORE  %d", score), 43, 177, 24, GOLD);
        DrawText(TextFormat("TARGET %d", TargetScore), 43, 211, 18, LIGHTGRAY);
        DrawText(TextFormat("SHOTS  %d", shotsLeft), 43, 250, 22, shotsLeft <= 6 ? ORANGE : RAYWHITE);
        DrawText(TextFormat("POPPED %d", poppedCount), 43, 284, 18, Color{ 120, 240, 170, 255 });
        DrawText(TextFormat("COMBO  x%d", std::max(1, combo)), 43, 316, 18, Color{ 220, 150, 255, 255 });

        DrawText("PRISM POWER", 34, 385, 16, LIGHTGRAY);
        for (int i = 0; i < 3; ++i)
        {
            const Color barColor = i < power ? Color{ 204, 103, 255, 255 } : Color{ 32, 53, 70, 255 };
            DrawRectangle(35 + i * 58, 413, 48, 14, barColor);
        }
        DrawText("3 COMBOS = WILD SHOT", 31, 441, 14, Color{ 190, 155, 224, 255 });

        const float progress = std::clamp(static_cast<float>(score) / TargetScore, 0.0f, 1.0f);
        const Rectangle progressBack{ width / 2.0f - 245.0f, GetScreenHeight() - 46.0f, 490.0f, 17.0f };
        DrawRectangleRounded(progressBack, 0.5f, 8, Color{ 20, 37, 52, 255 });
        DrawRectangleRounded(
            Rectangle{ progressBack.x, progressBack.y, progressBack.width * progress, progressBack.height },
            0.5f,
            8,
            Color{ 55, 225, 144, 255 }
        );
        DrawText("CHARGE THE NORTH GATE", width / 2 - MeasureText("CHARGE THE NORTH GATE", 16) / 2, GetScreenHeight() - 72, 16, RAYWHITE);

        if (messageTimer > 0.0f)
        {
            const char* text = "";
            Color color = RAYWHITE;
            switch (message)
            {
            case Message::Intro: text = "BUILD ANGLES, BREAK CLUSTERS, DROP HANGING ORBS"; color = Color{ 115, 230, 255, 255 }; break;
            case Message::Match: text = combo > 1 ? "CHAIN COMBO!" : "COLOR MATCH!"; color = LIME; break;
            case Message::BigDrop: text = "STRUCTURE COLLAPSE - HUGE BONUS!"; color = GOLD; break;
            case Message::Miss: text = "NO MATCH - PRESSURE IS BUILDING"; color = ORANGE; break;
            case Message::Pressure: text = "VAULT DESCENDS!"; color = RED; break;
            case Message::PrismReady: text = "PRISM SHOT READY - MATCHES ANY COLOR"; color = Color{ 225, 130, 255, 255 }; break;
            default: break;
            }
            DrawText(text, width / 2 - MeasureText(text, 20) / 2, 91, 20, color);
        }
    }

    void DrawPortal() const
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        DrawRectangle(0, 0, width, height, Color{ 0, 8, 15, 190 });

        const Rectangle portal = GetPortalRectangle();
        const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetTime()) * 4.0f);
        DrawRectangleRounded(
            Rectangle{ portal.x - 18.0f, portal.y - 18.0f, portal.width + 36.0f, portal.height + 36.0f },
            0.15f,
            12,
            Fade(Color{ 60, 255, 180, 255 }, 0.18f + pulse * 0.15f)
        );
        DrawRectangleRounded(portal, 0.15f, 12, Color{ 8, 51, 54, 255 });
        DrawRectangleRoundedLinesEx(portal, 0.15f, 12, 4.0f, Color{ 61, 255, 166, 255 });
        DrawText("NORTH GATE OPEN", width / 2 - MeasureText("NORTH GATE OPEN", 36) / 2, static_cast<int>(portal.y - 72.0f), 36, LIME);
        DrawText("ENTER THE GATE", width / 2 - MeasureText("ENTER THE GATE", 26) / 2, static_cast<int>(portal.y + 34.0f), 26, RAYWHITE);
        DrawText("CLICK THE GATE OR PRESS ENTER", width / 2 - MeasureText("CLICK THE GATE OR PRESS ENTER", 18) / 2, static_cast<int>(portal.y + 79.0f), 18, Color{ 150, 235, 220, 255 });
    }

    void DrawFailurePanel() const
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        DrawRectangle(0, 0, width, height, Color{ 0, 0, 0, 190 });
        DrawText("THE VAULT OVERLOADED", width / 2 - MeasureText("THE VAULT OVERLOADED", 42) / 2, height / 2 - 85, 42, RED);
        DrawText(TextFormat("SCORE %d / %d", score, TargetScore), width / 2 - MeasureText(TextFormat("SCORE %d / %d", score, TargetScore), 25) / 2, height / 2 - 18, 25, RAYWHITE);
        DrawText("PRESS R OR ENTER TO RETRY LEVEL 2", width / 2 - MeasureText("PRESS R OR ENTER TO RETRY LEVEL 2", 22) / 2, height / 2 + 42, 22, Color{ 112, 226, 255, 255 });
    }

    void DrawParticles() const
    {
        for (const Particle& particle : particles)
        {
            DrawCircleV(particle.position, particle.radius, Fade(particle.color, std::clamp(particle.life, 0.0f, 1.0f)));
        }
    }

    void DrawBackgroundEnergy() const
    {
        const float time = static_cast<float>(GetTime());
        for (int i = 0; i < 24; ++i)
        {
            const float x = std::fmod(i * 173.0f + time * (8.0f + i % 5), static_cast<float>(GetScreenWidth()));
            const float y = std::fmod(i * 97.0f + time * (13.0f + i % 4), static_cast<float>(GetScreenHeight()));
            DrawCircleV(Vector2{ x, y }, 2.0f + (i % 3), Fade(Color{ 80, 220, 255, 255 }, 0.18f));
        }
    }
};
