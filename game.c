#include <raylib.h>
#include <raymath.h>
#include <math.h>   // Required for cos() and sin()
#include <stdlib.h> // Required for GetRandomValue() on some platforms

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 500
#define MAX_PROJECTILE 100
#define MAX_ENEMIES 100
#define WINDOW_TITLE "SKYFIGHT"

static float chargeTime = 0.0f;

// Represents a single projectile fired by the player
typedef struct Projectile
{
    Vector2 pos;
    Vector2 dir;
    float speed;
    float size;
    int pierce;
} Projectile;

// Represents a single enemy
typedef struct Enemy
{
    Vector2 pos;
    float size;
    float speed;
} Enemy;

// Holds all the state for the current game
typedef struct GameState
{
    float gameTime;
    Vector2 playerPos;
    float playerSize;
    float playerSpeed;
    float shootDelay;
    float shootTime;
    float projspeed;
    float projsize;
    float projpierce;
    int projCount;
    Projectile projectile[MAX_PROJECTILE];
    float spawntime;
    int enemyCount;
    Enemy enemies[MAX_ENEMIES];
    int pointsScored;
    bool gameOver;
    bool gameStarted; // New field to track if the game has started
} GameState;

// Default state to initialize or reset the game
static const GameState DefaultState = {
    .gameTime = 0.0f,
    .playerPos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f},
    .playerSize = 20.0f,
    .playerSpeed = 250.0f,
    .shootDelay = 0.4f,
    .shootTime = 0.0f,
    .projspeed = 150.0f,
    .projsize = 4.0f,
    .projpierce = 1,
    .projCount = 0,
    .projectile = {0},
    .spawntime = 0.0f,
    .enemyCount = 0,
    .enemies = {0},
    .pointsScored = 0,
    .gameOver = false,
    .gameStarted = false}; // Initial state is the intro screen

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    GameState gamestate = DefaultState;
    // NOTE: Make sure your asset paths are correct
    Texture2D airplane = LoadTexture("build/assets/air.png");
    Texture2D enemyTex = LoadTexture("build/assets/enemies.png");

    // Main game loop
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color){0, 0, 0, 255});

        if (!gamestate.gameStarted)
        {
            // --- Intro Screen ---
            const char *titleText = "SKYFIGHT";
            int titleWidth = MeasureText(titleText, 80);
            DrawText(titleText, SCREEN_WIDTH / 2 - titleWidth / 2, SCREEN_HEIGHT / 2 - 150, 80, RAYWHITE);

            const char *controlsText1 = "W, A, S, D to Move";
            int controlsWidth1 = MeasureText(controlsText1, 20);
            DrawText(controlsText1, SCREEN_WIDTH / 2 - controlsWidth1 / 2, SCREEN_HEIGHT / 2 - 50, 20, LIGHTGRAY);

            const char *controlsText2 = "Hold X to charge and shoot towards the mouse";
            int controlsWidth2 = MeasureText(controlsText2, 20);
            DrawText(controlsText2, SCREEN_WIDTH / 2 - controlsWidth2 / 2, SCREEN_HEIGHT / 2 - 20, 20, LIGHTGRAY);

            const char *startText = "Press ENTER to Start";
            int startWidth = MeasureText(startText, 30);
            DrawText(startText, SCREEN_WIDTH / 2 - startWidth / 2, SCREEN_HEIGHT / 2 + 40, 30, WHITE);
            
            const char *creatorText = "Created by Rishit Mohanty";
            int creatorWidth = MeasureText(creatorText, 20);
            DrawText(creatorText, SCREEN_WIDTH / 2 - creatorWidth / 2, SCREEN_HEIGHT - 70, 20, GRAY);
            
            const char *githubText = "github.com/noneclashofclans";
            DrawText(githubText, 40, SCREEN_HEIGHT - 45, 40, GRAY);


            if (IsKeyPressed(KEY_ENTER))
            {
                gamestate.gameStarted = true;
            }
        }
        else if (!gamestate.gameOver)
        {
            gamestate.gameTime += GetFrameTime();

            // --- Player Input and Shooting ---
            if (gamestate.shootTime < gamestate.shootDelay)
                gamestate.shootTime += GetFrameTime();

            if (IsKeyDown(KEY_X))
            {
                Vector2 dir = Vector2Normalize(Vector2Subtract(GetMousePosition(), gamestate.playerPos));
                while (gamestate.shootTime >= gamestate.shootDelay)
                {
                    if (gamestate.projCount >= MAX_PROJECTILE)
                        break;
                    Projectile proj = {0};
                    proj.pos = gamestate.playerPos;
                    proj.dir = dir;
                    proj.speed = gamestate.projspeed;
                    proj.size = gamestate.projsize;
                    proj.pierce = (int)gamestate.projpierce;
                    gamestate.projectile[gamestate.projCount++] = proj;

                    chargeTime += GetFrameTime();
                    gamestate.projspeed = 450.0f + chargeTime * 200.0f; // Increase speed while charging
                    gamestate.shootTime -= gamestate.shootDelay;
                }
            }

            if (IsKeyReleased(KEY_X))
            {
                chargeTime = 0.0f;
                gamestate.projspeed = 150.0f; // Reset speed on release
            }

            // --- Player Movement ---
            if (IsKeyDown(KEY_A)) gamestate.playerPos.x -= gamestate.playerSpeed * GetFrameTime();
            if (IsKeyDown(KEY_D)) gamestate.playerPos.x += gamestate.playerSpeed * GetFrameTime();
            if (IsKeyDown(KEY_W)) gamestate.playerPos.y -= gamestate.playerSpeed * GetFrameTime();
            if (IsKeyDown(KEY_S)) gamestate.playerPos.y += gamestate.playerSpeed * GetFrameTime();

            // Clamp player position to stay within screen bounds
            gamestate.playerPos.x = Clamp(gamestate.playerPos.x, gamestate.playerSize, SCREEN_WIDTH - gamestate.playerSize);
            gamestate.playerPos.y = Clamp(gamestate.playerPos.y, gamestate.playerSize, SCREEN_HEIGHT - gamestate.playerSize);

            // --- Drawing Player ---
            if (airplane.id != 0)
            {
                float rotation = Vector2Angle((Vector2){1, 0}, Vector2Subtract(GetMousePosition(), gamestate.playerPos)) * RAD2DEG;
                DrawTexturePro(
                    airplane,
                    (Rectangle){0, 0, (float)airplane.width, (float)airplane.height},
                    (Rectangle){gamestate.playerPos.x, gamestate.playerPos.y, airplane.width / 4.0f, airplane.height / 4.0f},
                    (Vector2){(airplane.width / 8.0f), (airplane.height / 8.0f)}, // Center origin for rotation
                    rotation,
                    WHITE);
            }
            else
            {
                DrawCircle(gamestate.playerPos.x, gamestate.playerPos.y, gamestate.playerSize, GREEN);
                DrawText("Player texture missing", 10, SCREEN_HEIGHT - 30, 20, RED);
            }

            // --- Projectile Update and Collision ---
            for (int i = 0; i < gamestate.projCount; i++)
            {
                Projectile *proj = &gamestate.projectile[i];
                proj->pos = Vector2Add(proj->pos, Vector2Scale(proj->dir, proj->speed * GetFrameTime()));
                DrawCircle(proj->pos.x, proj->pos.y, proj->size, BLUE);

                // Check collision with enemies
                for (int j = 0; j < gamestate.enemyCount; j++)
                {
                    Enemy *enemy = &gamestate.enemies[j];
                    if (CheckCollisionCircles(proj->pos, proj->size, enemy->pos, enemy->size))
                    {
                        gamestate.pointsScored += 10;
                        // Remove enemy and projectile by swapping with the last element
                        gamestate.enemies[j] = gamestate.enemies[--gamestate.enemyCount];
                        gamestate.projectile[i] = gamestate.projectile[--gamestate.projCount];
                        i--; // Re-check the current index since a new projectile was moved here
                        break;
                    }
                }
            }

            // --- Player-Enemy Collision ---
            for (int i = 0; i < gamestate.enemyCount; i++)
            {
                if (CheckCollisionCircles(gamestate.playerPos, gamestate.playerSize, gamestate.enemies[i].pos, gamestate.enemies[i].size))
                {
                    gamestate.gameOver = true;
                    break;
                }
            }

            // --- Enemy Spawning ---
            float spawnInterval = Clamp(2.0f - gamestate.gameTime / 20.0f, 0.2f, 2.0f);
            gamestate.spawntime += GetFrameTime();
            while (gamestate.spawntime >= spawnInterval)
            {
                if (gamestate.enemyCount < MAX_ENEMIES)
                {
                    float radianValue = GetRandomValue(0, 360) * DEG2RAD;
                    float enemySize = Clamp(8 - gamestate.gameTime / 20.0f, 2.0f, 8.0f);
                    Vector2 dir = {cosf(radianValue), sinf(radianValue)};
                    float dist = SCREEN_WIDTH / 2.0f; // Spawn distance from player

                    Enemy enemy = {
                        .pos = {gamestate.playerPos.x + dir.x * dist, gamestate.playerPos.y + dir.y * dist},
                        .size = enemySize,
                        .speed = 60.0f + gamestate.gameTime * 2.0f // Speed increases over time
                    };
                    gamestate.enemies[gamestate.enemyCount++] = enemy;
                }
                gamestate.spawntime -= spawnInterval;
            }

            // --- Enemy Movement and Drawing ---
            for (int i = 0; i < gamestate.enemyCount; i++)
            {
                Enemy *e = &gamestate.enemies[i];
                Vector2 dir = Vector2Normalize(Vector2Subtract(gamestate.playerPos, e->pos));
                e->pos = Vector2Add(e->pos, Vector2Scale(dir, e->speed * GetFrameTime()));

                if (enemyTex.id != 0)
                {
                    DrawTexturePro(
                        enemyTex,
                        (Rectangle){0, 0, (float)enemyTex.width, (float)enemyTex.height},
                        (Rectangle){e->pos.x, e->pos.y, enemyTex.width / 8.0f, enemyTex.height / 8.0f},
                        (Vector2){(enemyTex.width / 16.0f), (enemyTex.height / 16.0f)}, // Center origin
                        0.0f,
                        WHITE);
                }
                else
                {
                    DrawCircle(e->pos.x, e->pos.y, e->size, RED);
                }
            }

            // --- UI Display ---
            if (enemyTex.id == 0)
            {
                DrawText("Enemy texture missing", 10, SCREEN_HEIGHT - 60, 20, RED);
            }
            DrawText(TextFormat("Score: %d", gamestate.pointsScored), 10, 10, 20, RAYWHITE);
            DrawText(TextFormat("Enemies: %d", gamestate.enemyCount), 10, 35, 20, RAYWHITE);
            DrawText(TextFormat("Projectiles: %d", gamestate.projCount), 10, 60, 20, RAYWHITE);
        }
        else // --- Game Over Screen ---
        {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 150});
            const char *gameOverText = "YOU DIED!";
            int gameOverWidth = MeasureText(gameOverText, 60);
            DrawText(gameOverText, SCREEN_WIDTH / 2 - gameOverWidth / 2, SCREEN_HEIGHT / 2 - 60, 60, RED);

            const char *scoreText = TextFormat("Final Score: %d", gamestate.pointsScored);
            int scoreWidth = MeasureText(scoreText, 30);
            DrawText(scoreText, SCREEN_WIDTH / 2 - scoreWidth / 2, SCREEN_HEIGHT / 2 + 20, 30, WHITE);

            const char *restartText = "Press R to Restart or ESC to Exit";
            int restartWidth = MeasureText(restartText, 20);
            DrawText(restartText, SCREEN_WIDTH / 2 - restartWidth / 2, SCREEN_HEIGHT / 2 + 80, 20, GRAY);

            if (IsKeyPressed(KEY_R))
            {
                gamestate = DefaultState; // Reset the game state
            }
        }

        EndDrawing();
    }

    // Unload textures and close the window
    UnloadTexture(airplane);
    UnloadTexture(enemyTex);
    CloseWindow();

    return 0;
}


