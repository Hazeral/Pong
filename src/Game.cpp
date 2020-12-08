#include "Game.h"

int screenWidth;
int screenHeight;

void GetDesktopResolution(int& width, int& height)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);

    width = desktop.right;
    height = desktop.bottom;
}

int centerCornerX(int winWidth) {
    return (screenWidth / 2) - (winWidth / 2);
}

int centerCornerY(int winHeight) {
    return (screenHeight / 2) - (winHeight / 2);
}

Game::Game(int width, int height) {
    mWinTitle = "Pong";
    mWinWidth = width;
    mWinHeight = height;

    mTicksCount = 0;
    mWallThickness = 15;
    mPaddleHeight = 100;
    mPaddleSpeed = 300.0f;
    mPaddlePos = { static_cast<float>(mWallThickness), static_cast<float>(mWinHeight / 2) }; // mWallThickness will give a slight margin from the left
    mBotPaddlePos = { static_cast<float>(mWinWidth - mWallThickness), static_cast<float>(mWinHeight / 2) };

    Ball ballOne = { 
        { static_cast<float>(mWinWidth / 2), static_cast<float>(mWinHeight / 2) },
        { -200.0f, 235.0f },
        10
    };
    
    mBalls = { 
        {
            {
                static_cast<float>(mWinWidth / 2),
                static_cast<float>(mWinHeight / 2)
            },
            { -200.0f, 235.0f }, 10
        },
        {
            {
                static_cast<float>(mWinWidth / 2),
                static_cast<float>(mWinHeight / 2)
            },
            { -200.0f, 200.0f }, 10
        }
    };
}

bool Game::Initialise() {
    int sdlResult = SDL_Init(SDL_INIT_VIDEO);

    GetDesktopResolution(screenWidth, screenHeight);

    mWindow = SDL_CreateWindow(mWinTitle, centerCornerX(mWinWidth), centerCornerY(mWinHeight), // 2ND and 3RD args are x, y coords for top-left corner of window
        mWinWidth, mWinHeight, 0); // last arg is flags

    mRenderer = SDL_CreateRenderer(mWindow, -1, // -1 lets SDL decide which graphics driver to use
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (sdlResult != 0) {
        SDL_Log("Error initialising SDL: %s", SDL_GetError());

        return false;
    }

    if (!mWindow) {
        SDL_Log("Error creating window: %s", SDL_GetError());

        return false;
    }

    if (!mRenderer) {
        SDL_Log("Error creating renderer: %s", SDL_GetError());

        return false;
    }

    mIsRunning = true;
    return true;
}

void Game::Shutdown() {
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void Game::RunLoop() {
    while (mIsRunning) {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::ProcessInput() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            mIsRunning = false;
            break;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(NULL);

    mPaddleDirection = 0;
    mBotPaddleDirection = 0;

    // -= and += so that it is stationary if both keys are held down
    if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP]) mPaddleDirection -= 1;
    if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN]) mPaddleDirection += 1;

    // Bot movement
    Ball* closestBall = &mBalls[0];

    for (size_t i = 1; i < mBalls.size(); i++) {
        Ball* ball = &mBalls[i];

        if (ball->Position.x > closestBall->Position.x) closestBall = ball;
    }

    if (closestBall->Position.x > mWinWidth - (mWinWidth / 3) && closestBall->Velocity.x > 0) {
        if (mBotPaddlePos.y < closestBall->Position.y) mBotPaddleDirection += 1;
        else if (mBotPaddlePos.y > closestBall->Position.y) mBotPaddleDirection -= 1;
    }
}

void Game::UpdateGame() {
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)); // 60FPS limit

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f; // Divided by 1k to get seconds
    mTicksCount = SDL_GetTicks();

    if (deltaTime > 0.05f) deltaTime = 0.05f; // Limit max delta time to prevent jumping frames (i.e. on breakpoints)

    if (mPaddleDirection != 0) {
        mPaddlePos.y += mPaddleDirection * mPaddleSpeed * deltaTime;

        if (mPaddlePos.y < (mPaddleHeight / 2.0f) + mWallThickness) { // (mPaddleHeight / 2.0f) + mWallThickness is the top boundary
            mPaddlePos.y = (mPaddleHeight / 2.0f) + mWallThickness;
        }
        else if (mPaddlePos.y > mWinHeight - (mPaddleHeight / 2.0f) - mWallThickness) {
            mPaddlePos.y = mWinHeight - (mPaddleHeight / 2.0f) - mWallThickness;
        }
    }

    if (mBotPaddleDirection != 0) {
        mBotPaddlePos.y += mBotPaddleDirection * mPaddleSpeed * deltaTime;

        if (mBotPaddlePos.y < (mPaddleHeight / 2.0f) + mWallThickness) { // (mPaddleHeight / 2.0f) + mWallThickness is the top boundary
            mBotPaddlePos.y = (mPaddleHeight / 2.0f) + mWallThickness;
        }
        else if (mBotPaddlePos.y > mWinHeight - (mPaddleHeight / 2.0f) - mWallThickness) {
            mBotPaddlePos.y = mWinHeight - (mPaddleHeight / 2.0f) - mWallThickness;
        }
    }

    for (size_t i = 0; i < mBalls.size(); i++) {
        Ball* ball = &mBalls[i];

        ball->Position.x += ball->Velocity.x * deltaTime;
        ball->Position.y += ball->Velocity.y * deltaTime;

        // Ball collisions
        if (ball->Position.y - ball->Radius <= mWallThickness || ball->Position.y + ball->Radius >= mWinHeight - mWallThickness)
        {
            ball->Velocity.y *= -1;
        }
        else if ( // This could probably be significantly improved
            (
                (
                    ( // Ball is within the paddle
                        ball->Position.y + ball->Radius <= mPaddlePos.y + (mPaddleHeight / 2) &&
                        ball->Position.y - ball->Radius >= mPaddlePos.y - (mPaddleHeight / 2)
                        ) ||
                    ( // Bottom edge of ball in paddle
                        ball->Position.y + ball->Radius <= mPaddlePos.y + (mPaddleHeight / 2) &&
                        ball->Position.y + ball->Radius >= mPaddlePos.y - (mPaddleHeight / 2)
                        ) ||
                    ( // Top edge of ball in paddle
                        ball->Position.y - ball->Radius >= mPaddlePos.y - (mPaddleHeight / 2) &&
                        ball->Position.y - ball->Radius <= mPaddlePos.y + (mPaddleHeight / 2)
                        )
                    ) &&
                ball->Position.x <= mPaddlePos.x + mWallThickness &&
                ball->Position.x >= mPaddlePos.x - (mWallThickness / 2.0f) &&
                ball->Velocity.x < 0.0f
                ) ||
            ( // Bot paddle
                (
                    (
                        ball->Position.y + ball->Radius <= mBotPaddlePos.y + (mPaddleHeight / 2) &&
                        ball->Position.y - ball->Radius >= mBotPaddlePos.y - (mPaddleHeight / 2)
                        ) ||
                    (
                        ball->Position.y + ball->Radius <= mBotPaddlePos.y + (mPaddleHeight / 2) &&
                        ball->Position.y + ball->Radius >= mBotPaddlePos.y - (mPaddleHeight / 2)
                        ) ||
                    (
                        ball->Position.y - ball->Radius >= mBotPaddlePos.y - (mPaddleHeight / 2) &&
                        ball->Position.y - ball->Radius <= mBotPaddlePos.y + (mPaddleHeight / 2)
                        )
                    ) &&
                ball->Position.x <= mBotPaddlePos.x + (mWallThickness / 2.0f) &&
                ball->Position.x >= mBotPaddlePos.x - (mWallThickness * 1.5f) &&
                ball->Velocity.x > 0.0f
                )
            )
        {
            ball->Velocity.x *= -1;
        }

        if (ball->Position.x < 0 || ball->Position.x > mWinWidth) ball->Position = { static_cast<float>(mWinWidth / 2), static_cast<float>(mWinHeight / 2) }; // Reset ball if out of bounds
    }
}

void Game::GenerateOutput() {
    // Background colour
    SDL_SetRenderDrawColor(
        mRenderer,
        0, 0, 0, 255 // rgba
    );

    SDL_RenderClear(mRenderer); // Clear back buffer to current draw colour

    // Render scene here
    SDL_Rect topWall{
        0, 0, // Top left corner xy
        mWinWidth, // width
        mWallThickness // height
    };

    SDL_Rect bottomWall{ 0, mWinHeight - mWallThickness, mWinWidth, mWallThickness };

    SDL_Rect paddle{
        static_cast<int>(mPaddlePos.x - (mWallThickness / 2)),
        static_cast<int>(mPaddlePos.y - (mPaddleHeight / 2)),
        mWallThickness, mPaddleHeight };

    SDL_Rect bot {
        static_cast<int>(mBotPaddlePos.x - (mWallThickness / 2)),
        static_cast<int>(mBotPaddlePos.y - (mPaddleHeight / 2)),
        mWallThickness, mPaddleHeight };

    SDL_SetRenderDrawColor(mRenderer, 0, 0, 255, 255);
    SDL_RenderFillRect(mRenderer, &topWall);
    SDL_RenderFillRect(mRenderer, &bottomWall);

    SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255);
    SDL_RenderFillRect(mRenderer, &paddle);
    SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);
    SDL_RenderFillRect(mRenderer, &bot);

    SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
    for (size_t i = 0; i < mBalls.size(); i++) {
        Ball* ball = &mBalls[i];

        SDL_Rect ballRect {
            static_cast<int>(ball->Position.x - (mWallThickness / 2)),
            static_cast<int>(ball->Position.y - (mWallThickness / 2)),
            ball->Radius * 2, ball->Radius * 2 };

        SDL_RenderFillRect(mRenderer, &ballRect);
    }

    SDL_RenderPresent(mRenderer); // Swap the two colour buffers
}