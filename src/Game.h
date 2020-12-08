#pragma once
#include <SDL/SDL.h>

struct Vector2 {
    float x;
    float y;
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
    int mBallRadius;
    int mPaddleHeight;
    int mPaddleDirection; // 0 = stationary, -1 = up, 1 = down
    int mBotPaddleDirection;
    float mPaddleSpeed; // pixels/second
    // Center anchor
    Vector2 mPaddlePos;
    Vector2 mBotPaddlePos;
    Vector2 mBallPos;
    Vector2 mBallVelocity;
};

