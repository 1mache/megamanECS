#include "megaman.h"
#include "GlobalData.h"
#include "MTransform.h"
#include <cmath>
#include <iostream>

namespace
{
constexpr float SPRITE_W = 28.f;
constexpr float SPRITE_H = 28.f;
constexpr int   RUN_START = 0;
constexpr int   RUN_COUNT = 4;
constexpr int   IDLE_START = 8;
constexpr int   IDLE_COUNT = 1;
constexpr int   JUMP_START = 10;
constexpr int   JUMP_COUNT = 1;
constexpr int   ANIM_SPEED = 8; // frames between animation ticks
} // namespace

namespace megaman
{
ent_type createPlayer(float x, float y, int hp)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<Animation>(ent, {});
    bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
    bagel::World::addComponent<MTransform>(
        ent,
        {.x = x,
         .y = y,
         .w = SPRITE_W / (2 * GlobalData::PTM),
         .h = SPRITE_H / (2 * GlobalData::PTM)});
    bagel::World::addComponent<Movement>(ent, {.mass = 1});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = hp});
    bagel::World::addComponent<Input>(ent, {});
    bagel::World::addComponent<Weapon>(ent, {});

    return ent;
}

ent_type createEnemy(float x, float y, int hp)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});
    bagel::World::addComponent<Movement>(ent, {.mass = 1});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = hp});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<AI>(ent, {.state = -1});

    return ent;
}

ent_type createBoss(float x, float y, int hp)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});
    bagel::World::addComponent<Movement>(ent, {.mass = 1});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = hp});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<AI>(ent, {.state = -1});
    // based on boss type
    bagel::World::addComponent<Weapon>(ent, {.projectileType = -1});

    return ent;
}

ent_type createPlatform(float x, float y, bool isMoving)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});
    bagel::World::addComponent<Collision>(ent, {});
    if (isMoving)
        bagel::World::addComponent<Movement>(ent, {.mass = 0});

    return ent;
}

ent_type createProjectile(float x, float y, float velX, float velY)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});
    bagel::World::addComponent<Movement>(
        ent,
        {.mass = 1, .velX = velX, .velY = velY});
    bagel::World::addComponent<Collision>(ent, {});

    return ent;
}

ent_type createTrigger(float x, float y, float width, float height)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});
    bagel::World::addComponent<Collision>(ent,
                                          {.width = width, .height = height});

    return ent;
}

ent_type createItem(float x, float y)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});
    bagel::World::addComponent<Collision>(ent, {});

    return ent;
}

ent_type createText(float x, float y, const std::string& text)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<Drawable>(
        ent,
        {.texture = nullptr}); // tie it to the text later somehow
    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});

    return ent;
}

ent_type createSoundSource(float x, float y, int sound)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});
    bagel::World::addComponent<Sound>(ent, {.sound = sound});

    return ent;
}

// ============= SYSTEMS =============

void InputSystem::run()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Input>().set<Movement>().build();

    SDL_PumpEvents();
    const bool* keys = SDL_GetKeyboardState(nullptr);

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(mask))
        {
            auto& m = e.get<Movement>();
            m.velX = 0.f;
            if (keys[SDL_SCANCODE_LEFT])
                m.velX = -3.f;
            if (keys[SDL_SCANCODE_RIGHT])
                m.velX = 3.f;
            if (keys[SDL_SCANCODE_UP])
                m.velY = -5.f;
        }
    }
}

void MovementSystem::run()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<MTransform>().set<Movement>().build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(mask))
        {
            auto&       t = e.get<MTransform>();
            const auto& m = e.get<Movement>();
            t.x += m.velX;
            t.y += m.velY;

            GlobalData::updateCamPosition(t.x, t.y);
        }
    }
}

void AnimationSystem::run()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Animation>().set<Movement>().build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(mask))
        {
            auto&       a = e.get<Animation>();
            const auto& m = e.get<Movement>();

            const Animation::State newState =
                m.velX != 0.f ? Animation::RUN : Animation::IDLE;

            if (newState != a.state)
            {
                a.state = newState;
                a.currentFrame = 0;
                a.frameTimer = 0;
            }

            if (++a.frameTimer >= ANIM_SPEED)
            {
                a.frameTimer = 0;
                ++a.currentFrame;
            }
        }
    }
}

void DrawingSystem::run(SDL_Renderer* ren, SDL_Texture* tex)
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<MTransform>()
                                        .set<Drawable>()
                                        .set<Animation>()
                                        .build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(mask))
        {
            const auto& t = e.get<MTransform>();
            const auto& a = e.get<Animation>();

            int startFrame, frameCount;
            switch (a.state)
            {
            case Animation::RUN:
                startFrame = RUN_START;
                frameCount = RUN_COUNT;
                break;
            case Animation::JUMP:
                startFrame = JUMP_START;
                frameCount = JUMP_COUNT;
                break;
            default:
                startFrame = IDLE_START;
                frameCount = IDLE_COUNT;
                break;
            }

            const int frame = startFrame + (a.currentFrame % frameCount);
            SDL_FRect src = {frame * SPRITE_W, 0.f, SPRITE_W, SPRITE_H};
            SDL_FRect dest = transformToFrect(t);

            std::cout << "<" << dest.x << ',' << dest.y << ">\n";

            SDL_RenderTexture(ren, tex, &src, &dest);
        }
    }
}

void CollisionSystem::run(b2WorldId)
{
}
void HealthSystem::run()
{
}
void AISystem::run()
{
}
void SoundSystem::run()
{
}
} // namespace megaman
