#include "megamanGame.h"
#include "GlobalData.h"
#include "megaman.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>

namespace
{
constexpr float PLAYER_SPRITE_W = 28.f;
constexpr float PLAYER_SPRITE_H = 28.f;
constexpr float PLAYER_SCALE = 3.f;
constexpr int   PLAYER_IDLE_START = 8;
constexpr int   PLAYER_IDLE_COUNT = 1;
constexpr int   PLAYER_RUN_START = 0;
constexpr int   PLAYER_RUN_COUNT = 4;
constexpr int   PLAYER_JUMP_START = 10;
constexpr int   PLAYER_JUMP_COUNT = 1;

constexpr float ENEMY_SPRITE_W = 24.f;
constexpr float ENEMY_SPRITE_H = 24.f;
constexpr float ENEMY_SCALE = 2.f;
constexpr int   ENEMY_IDLE_START = 0;
constexpr int   ENEMY_IDLE_COUNT = 1;
constexpr int   ENEMY_RUN_START = 0;
constexpr int   ENEMY_RUN_COUNT = 2;
constexpr int   ENEMY_JUMP_START = 0;
constexpr int   ENEMY_JUMP_COUNT = 1;

constexpr float LOCKSTER_SPRITE_W = 24.f;
constexpr float LOCKSTER_SPRITE_H = 24.f;
constexpr float LOCKSTER_SCALE = 2.f;
constexpr int   LOCKSTER_IDLE_START = 0;
constexpr int   LOCKSTER_IDLE_COUNT = 2;
constexpr int   LOCKSTER_ALERT_START = 2;
constexpr int   LOCKSTER_ALERT_COUNT = 4;
constexpr int   LOCKSTER_CHARGE_START = 6;
constexpr int   LOCKSTER_CHARGE_COUNT = 2;
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

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0, -GlobalData::GRAVITY};
    _box = b2CreateWorld(&worldDef);

    _scene.load(_ren);
    _scene.attachPhysics(_box);

    // TODO: take position from _scene
    ent_type player = createPlayer(_box, 14, 4.5f, MegamanGame::HP);
    {
        auto& d = bagel::World::getComponent<Drawable>(player);
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

    ent_type patrolingEnemy =
        createPatroller(_box, 3.f, 4.5f, MegamanGame::HP, 1.f, 5.f, 6.f, 0.05f);
    {
        auto& d = bagel::World::getComponent<Drawable>(patrolingEnemy);
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

    ent_type lockster =
        createLockster(_box, 40.f, 4.5f, MegamanGame::HP, 15.f, 0.18f);
    {
        auto& d = bagel::World::getComponent<Drawable>(lockster);
        d.texture = _locksterTex;
        d.spriteW = LOCKSTER_SPRITE_W;
        d.spriteH = LOCKSTER_SPRITE_H;
        d.drawScale = LOCKSTER_SCALE;
        d.idleStart = LOCKSTER_IDLE_START;
        d.idleCount = LOCKSTER_IDLE_COUNT;
        d.jumpStart = LOCKSTER_ALERT_START;
        d.jumpCount = LOCKSTER_ALERT_COUNT;
        d.runStart = LOCKSTER_CHARGE_START;
        d.runCount = LOCKSTER_CHARGE_COUNT;
        d.defaultFacingLeft = false;
    }
}

MegamanGame::~MegamanGame()
{
    if (b2World_IsValid(_box))
        b2DestroyWorld(_box);
    if (_tex != nullptr)
        SDL_DestroyTexture(_tex);
    if (_enemyTex != nullptr)
        SDL_DestroyTexture(_enemyTex);
    if (_locksterTex != nullptr)
        SDL_DestroyTexture(_locksterTex);
    if (_shotTex != nullptr)
        SDL_DestroyTexture(_shotTex);
    if (_explosionTex != nullptr)
        SDL_DestroyTexture(_explosionTex);
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
void MegamanGame::shootingSystem()
{
    ShootingSystem::run();
}
void MegamanGame::boxSystem()
{
    CollisionSystem::run(_box);
}
void MegamanGame::damageSystem()
{
    DamageSystem::run();
}
void MegamanGame::drawSystem()
{
    DrawingSystem::run(_ren);
}
void MegamanGame::animationSystem()
{
    AnimationSystem::run();
}
void MegamanGame::aiSystem()
{
    AISystem::run();
}
void MegamanGame::healthSystem()
{
    HealthSystem::run();
}
void MegamanGame::respawnSystem()
{
    RespawnSystem::run();
}
void MegamanGame::explosionSystem()
{
    ExplosionSystem::run();
}

void MegamanGame::run()
{
    auto start = SDL_GetTicks();
    bool quit = false;

    while (!quit)
    {
        inputSystem();
        aiSystem();
        moveSystem();
        shootingSystem();
        boxSystem();
        damageSystem();
        healthSystem();
        explosionSystem();
        respawnSystem();
        animationSystem();

        if (_scene.isValid())
        {
            CameraData cam = GlobalData::getCamData();
            _scene.clampCameraToBounds(cam);
            GlobalData::updateCamPosition(cam.posX, cam.posY);
        }

        SDL_RenderClear(_ren);
        if (_scene.isValid())
            _scene.draw(_ren, GlobalData::getCamData());

        drawSystem();
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
