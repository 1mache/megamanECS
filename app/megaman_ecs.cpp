#include "MegamanGame.h"

int main()
{
    megaman::MegamanGame game;
    if (game.valid())
        game.run();

    return 0;
}