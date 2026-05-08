#include "megamanGame.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>

namespace
{
    constexpr float PLAYER_SPRITE_W = 28.f;
    constexpr float PLAYER_SPRITE_H = 28.f;
    constexpr float PLAYER_SCALE = 3.f;
    constexpr int PLAYER_IDLE_START = 8;
    constexpr int PLAYER_IDLE_COUNT = 1;
    constexpr int PLAYER_RUN_START = 0;
    constexpr int PLAYER_RUN_COUNT = 4;
    constexpr int PLAYER_JUMP_START = 10;
    constexpr int PLAYER_JUMP_COUNT = 1;

    constexpr float ENEMY_SPRITE_W = 22.f;
    constexpr float ENEMY_SPRITE_H = 24.f;
    constexpr float ENEMY_SCALE = 2.f;
    constexpr int ENEMY_IDLE_START = 0;
    constexpr int ENEMY_IDLE_COUNT = 1;
    constexpr int ENEMY_RUN_START = 0;
    constexpr int ENEMY_RUN_COUNT = 2;
    constexpr int ENEMY_JUMP_START = 0;
    constexpr int ENEMY_JUMP_COUNT = 1;
} // namespace

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

        SDL_SetRenderScale(_ren, GlobalData::SCALE_FACTOR, GlobalData::SCALE_FACTOR);

        SDL_Surface *surf = IMG_Load("res/player.png"); // The mockup app now TODO: load the new level
        if (surf == nullptr)
        {
            std::cout << SDL_GetError() << std::endl;
            return;
        }

        _tex = SDL_CreateTextureFromSurface(_ren, surf);
        SDL_DestroySurface(surf);

        SDL_Surface *enemySurf = IMG_Load("res/enemy.png");
        if (enemySurf == nullptr)
        {
            std::cout << SDL_GetError() << std::endl;
            return;
        }

        _enemyTex = SDL_CreateTextureFromSurface(_ren, enemySurf);
        SDL_DestroySurface(enemySurf);

        if (_tex == nullptr)
        {
            std::cout << SDL_GetError() << std::endl;
            return;
        }

        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0, 20.f};
        _box = b2CreateWorld(&worldDef);

        ent_type player = createPlayer(6.f, 4.5f, MegamanGame::HP);
        {
            auto &d = bagel::World::getComponent<Drawable>(player);
            d.texture = _tex;
            d.spriteW = PLAYER_SPRITE_W;
            d.spriteH = PLAYER_SPRITE_H;
            d.drawScale = PLAYER_SCALE;
            d.idleStart = PLAYER_IDLE_START;
            d.idleCount = PLAYER_IDLE_COUNT;
            d.runStart = PLAYER_RUN_START;
            d.runCount = PLAYER_RUN_COUNT;
            d.jumpStart = PLAYER_JUMP_START;
            d.jumpCount = PLAYER_JUMP_COUNT;
        }

        ent_type patrolingEnemy = createPatroller(3.f, 4.5f, MegamanGame::HP, 1.f, 5.f, 6.f, 0.05f);
        {
            auto &d = bagel::World::getComponent<Drawable>(patrolingEnemy);
            d.texture = _enemyTex;
            d.spriteW = ENEMY_SPRITE_W;
            d.spriteH = ENEMY_SPRITE_H;
            d.drawScale = ENEMY_SCALE;
            d.idleStart = ENEMY_IDLE_START;
            d.idleCount = ENEMY_IDLE_COUNT;
            d.runStart = ENEMY_RUN_START;
            d.runCount = ENEMY_RUN_COUNT;
            d.jumpStart = ENEMY_JUMP_START;
            d.jumpCount = 1;
            d.defaultFacingLeft = false;
        }
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
        if (_enemyTex != nullptr)
            SDL_DestroyTexture(_enemyTex);

        SDL_Quit();
    }

    void MegamanGame::inputSystem() { InputSystem::run(); }
    void MegamanGame::moveSystem() { MovementSystem::run(); }
    void MegamanGame::boxSystem() { CollisionSystem::run(_box); }
    void MegamanGame::drawSystem() { DrawingSystem::run(_ren); }
    void MegamanGame::animationSystem() { AnimationSystem::run(); }
    void MegamanGame::aiSystem() { AISystem::run(); }

    void MegamanGame::run()
    {
        auto start = SDL_GetTicks();
        bool quit = false;

        while (!quit)
        {
            inputSystem();
            aiSystem();
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
                    (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_ESCAPE))
                    quit = true;
            }
        }
    }

}