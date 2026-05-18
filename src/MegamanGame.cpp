#include "MegamanGame.h"
#include "GlobalData.h"
#include "Megaman.h"
#include "Utils.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
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
    // good for pixel art scaling
    SDL_SetDefaultTextureScaleMode(_ren, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderDrawColor(_ren, 49, 49, 49, 255);

    GlobalData::setWindow(_win);
    GlobalData::setRenderer(_ren);

    _tex = IMG_LoadTexture(_ren, PLAYER_TEXTURE_PATH);
    if (_tex == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    _enemyTex = IMG_LoadTexture(_ren, "res/enemy.png");
    if (_enemyTex == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    _locksterTex = IMG_LoadTexture(_ren, "res/lockster.png");
    if (_locksterTex == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    _bossTex = IMG_LoadTexture(_ren, "res/boss.png");
    if (_bossTex == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    _shotTex = IMG_LoadTexture(_ren, "res/shot.png");
    if (_shotTex == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }
    GlobalData::setShotTexture(_shotTex);

    _explosionTex = IMG_LoadTexture(_ren, "res/explosion.png");
    if (_explosionTex == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }
    GlobalData::setExplosionTexture(_explosionTex);

    _heartTex = IMG_LoadTexture(_ren, "res/heart.png");
    if (_heartTex == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity    = {0, -GlobalData::GRAVITY};
    _boxWorld           = b2CreateWorld(&worldDef);
    GlobalData::setBoxWorld(_boxWorld);

    _scene.load(_ren);
    _scene.attachPhysics(_boxWorld);

    const auto& playerSpawns = _scene.getPlayerSpawns();
    auto& playersp = playerSpawns[playerSpawns.size() - 1]; // TODO: change to 0.
    createPlayer(_boxWorld, playersp.x, playersp.y, _tex);

    for (const SpawnPoint& sp : _scene.getEnemySpawns())
    {
        if (sp.type == SpawnPoint::Type::Ptrol)
        {
            createPatroller(_boxWorld,
                            sp.x,
                            sp.y,
                            std::min(sp.x, sp.patrolDestX),
                            std::max(sp.x, sp.patrolDestX),
                            _enemyTex);
        }
        else if (sp.type == SpawnPoint::Type::Lockster)
        {
            createLockster(_boxWorld, sp.x, sp.y, _locksterTex);
        }
        else if (sp.type == SpawnPoint::Type::Boss)
        {
            createBoss(_boxWorld, sp.x, sp.y, _bossTex);
        }
    }
}

MegamanGame::~MegamanGame()
{
    if (b2World_IsValid(_boxWorld))
        b2DestroyWorld(_boxWorld);
    if (_tex != nullptr)
        SDL_DestroyTexture(_tex);
    if (_enemyTex != nullptr)
        SDL_DestroyTexture(_enemyTex);
    if (_locksterTex != nullptr)
        SDL_DestroyTexture(_locksterTex);
    if (_bossTex != nullptr)
        SDL_DestroyTexture(_bossTex);
    if (_shotTex != nullptr)
        SDL_DestroyTexture(_shotTex);
    if (_explosionTex != nullptr)
        SDL_DestroyTexture(_explosionTex);
    if (_heartTex != nullptr)
        SDL_DestroyTexture(_heartTex);
    if (_ren != nullptr)
        SDL_DestroyRenderer(_ren);
    if (_win != nullptr)
        SDL_DestroyWindow(_win);

    SDL_Quit();
}

void MegamanGame::run()
{
    auto start = SDL_GetTicks();
    bool quit  = false;

    const float                    sceneMinY   = _scene.getBoundsM().minY;
    const std::vector<SpawnPoint>& checkpoints = _scene.getPlayerSpawns();
    const std::vector<SpawnPoint>  enemySpawns = _scene.getEnemySpawns();

    while (!quit)
    {
        inputSystem();
        aiSystem();
        bossSystem();
        jumpSystem();
        movementSystem(sceneMinY);
        checkpointSystem(checkpoints);
        shootingSystem();
        collisionSystem(_boxWorld);
        damageSystem();
        healthSystem();
        respawnSystem(_boxWorld, enemySpawns, _locksterTex);
        playerAnimSystem();
        patrollerAnimSystem();
        locksterAnimSystem();
        bossAnimSystem();
        explosionAnimSystem();

        if (_scene.isValid())
        {
            CameraData cam = GlobalData::getCamData();
            _scene.clampCameraToBounds(cam);
            GlobalData::updateCamPosition(cam.posX, cam.posY);
        }

        projectileCullSystem();

        SDL_RenderClear(_ren);
        if (_scene.isValid())
            _scene.draw(_ren, GlobalData::getCamData());

        drawSystem(_ren);
        hudSystem(_ren, _heartTex);
        SDL_RenderPresent(_ren);

        const auto end = SDL_GetTicks();
        if (end - start < GAME_FRAME)
            SDL_Delay(static_cast<Uint32>(GAME_FRAME - (end - start)));

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
