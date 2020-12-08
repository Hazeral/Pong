#pragma once
#include <SDL/SDL.h>
#include <wtypes.h>
#include <vector>

struct Vector2 {
    float x;
    float y;
};

struct Ball {
    Vector2 Position;
    Vector2 Velocity;
    int Radius;
};

class Game
{
public:
    Game(int width, int height);

    bool Initialise();
    void RunLoop();
    void Shutdown();

private:
    // called on each interation of the game loop (each frame)
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();

    Uint32 mTicksCount;
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    bool mIsRunning;
    const char* mWinTitle;
    int mWinWidth;
    int mWinHeight;
    int mWallThickness;
    int mPaddleHeight;
    int mPaddleDirection; // 0 = stationary, -1 = up, 1 = down
    int mBotPaddleDirection;
    float mPaddleSpeed; // pixels/second
    // Center anchor
    Vector2 mPaddlePos;
    Vector2 mBotPaddlePos;
    
    std::vector<Ball> mBalls;
};

