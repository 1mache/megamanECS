#include "megamanGame.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>

namespace megaman
{
MegamanGame::MegamanGame()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    if (!SDL_CreateWindowAndRenderer("Megaman", WIN_W, WIN_H, 0, &_win, &_ren))
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    SDL_SetRenderScale(_ren,
                       GlobalData::SCALE_FACTOR,
                       GlobalData::SCALE_FACTOR);

    SDL_Surface* surf = IMG_Load(
        "res/player.png"); // The mockup app now TODO: load the new level
    if (surf == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    _tex = SDL_CreateTextureFromSurface(_ren, surf);
    SDL_DestroySurface(surf);
    if (_tex == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0, 20.f};
    _box = b2CreateWorld(&worldDef);

    // SDL_Frect cameraCenter();

    createPlayer(1, 1, MegamanGame::HP);
}

MegamanGame::~MegamanGame()
{
    if (b2World_IsValid(_box))
        b2DestroyWorld(_box);
    if (_tex != nullptr)
        SDL_DestroyTexture(_tex);
    if (_ren != nullptr)
        SDL_DestroyRenderer(_ren);
    if (_win != nullptr)
        SDL_DestroyWindow(_win);

    SDL_Quit();
}

void MegamanGame::inputSystem()
{
    InputSystem::run();
}
void MegamanGame::moveSystem()
{
    MovementSystem::run();
}
void MegamanGame::boxSystem()
{
    CollisionSystem::run(_box);
}
void MegamanGame::drawSystem()
{
    DrawingSystem::run(_ren, _tex);
}
void MegamanGame::animationSystem()
{
    AnimationSystem::run();
}

void MegamanGame::run()
{
    auto start = SDL_GetTicks();
    bool quit = false;

    while (!quit)
    {
        inputSystem();
        animationSystem();
        moveSystem();
        boxSystem();

        SDL_RenderClear(_ren);
        drawSystem();
        SDL_RenderPresent(_ren);

        const auto end = SDL_GetTicks();
        if (end - start < GAME_FRAME)
        {
            SDL_Delay(static_cast<Uint32>(GAME_FRAME - (end - start)));
        }

        start += GAME_FRAME;

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if ((e.type == SDL_EVENT_QUIT) ||
                (e.type == SDL_EVENT_KEY_DOWN &&
                 e.key.scancode == SDL_SCANCODE_ESCAPE))
                quit = true;
        }
    }
}

} // namespace megaman