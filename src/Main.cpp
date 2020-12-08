#include "Game.h"

int main(int argc, char* args[]) {
    Game game(1125, 635);
    bool success = game.Initialise();

    if (success) {
        game.RunLoop();
    }

    game.Shutdown();

    return 0;
}