#include "megaman.h"
#include <cmath>
#include <iostream>

namespace
{
    constexpr float SPRITE_W = 28.f;
    constexpr float SPRITE_H = 28.f;
    constexpr float DRAW_SCALE = 3.f;
    constexpr int RUN_START = 0;
    constexpr int RUN_COUNT = 4;
    constexpr int IDLE_START = 8;
    constexpr int IDLE_COUNT = 1;
    constexpr int JUMP_START = 10;
    constexpr int JUMP_COUNT = 1;
    constexpr int ANIM_SPEED = 8; // frames between animation ticks
} // namespace

namespace megaman
{
    ent_type createPlayer(float x, float y, int hp)
    {
        ent_type ent = bagel::World::createEntity();

        bagel::World::addComponent<Animation>(ent, {});
        bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
        constexpr float hw = SPRITE_W * DRAW_SCALE / (2.f * GlobalData::PTM);
        constexpr float hh = SPRITE_H * DRAW_SCALE / (2.f * GlobalData::PTM);
        bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y, .w = hw, .h = hh});
        bagel::World::addComponent<Movement>(ent, {.mass = 1});
        bagel::World::addComponent<Collision>(ent, {});
        bagel::World::addComponent<Health>(ent, {.points = hp});
        bagel::World::addComponent<Input>(ent, {});
        bagel::World::addComponent<Weapon>(ent, {});

        return ent;
    }

    ent_type createPatroller(float x, float y, int hp, float patrolMinX, float patrolMaxX, float detectionRange, float speed)
    {
        ent_type ent = bagel::World::createEntity();

        bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
        bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y, .w = 0.733f, .h = 0.8f});
        bagel::World::addComponent<Movement>(ent, {.mass = 1});
        bagel::World::addComponent<Collision>(ent, {});
        bagel::World::addComponent<Health>(ent, {.points = hp});
        bagel::World::addComponent<Enemy>(ent, {});
        bagel::World::addComponent<AI>(ent, {.type = AI::Type::Patroller,
                                             .patrolMinX = patrolMinX,
                                             .patrolMaxX = patrolMaxX,
                                             .detectionRange = detectionRange,
                                             .speed = speed});
        bagel::World::addComponent<Animation>(ent, {});

        return ent;
    }

    ent_type createBoss(float x, float y, int hp) // is boss is nesserary or enemy is enough?
    {
        ent_type ent = bagel::World::createEntity();

        bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
        bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});
        bagel::World::addComponent<Movement>(ent, {.mass = 1});
        bagel::World::addComponent<Collision>(ent, {});
        bagel::World::addComponent<Health>(ent, {.points = hp});
        bagel::World::addComponent<Enemy>(ent, {});
        bagel::World::addComponent<AI>(ent, {});
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

    ent_type createText(float x, float y, const std::string &text)
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

    ent_type createScene(const std::string &mapFilePath)
    {
        ent_type ent = bagel::World::createEntity();

        bagel::World::addComponent<Scene>(ent, {.mapFilePath = mapFilePath});

        return ent;
    }

    // ============= SYSTEMS =============

    void InputSystem::run()
    {
        static const bagel::Mask mask =
            bagel::MaskBuilder().set<Input>().set<Movement>().build();

        SDL_PumpEvents();
        const bool *keys = SDL_GetKeyboardState(nullptr);

        for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
        {
            if (e.test(mask))
            {
                auto &m = e.get<Movement>();
                m.velX = 0.f;
                m.velY = 0.f;
                if (keys[SDL_SCANCODE_LEFT])
                {
                    m.velX = -0.05f;
                    m.facingLeft = true;
                }
                if (keys[SDL_SCANCODE_RIGHT])
                {
                    m.velX = 0.05f;
                    m.facingLeft = false;
                }
                if (keys[SDL_SCANCODE_UP])
                    m.velY = 0.083f;
                if (keys[SDL_SCANCODE_DOWN])
                    m.velY = -0.083f;
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
                auto &t = e.get<MTransform>();
                const auto &m = e.get<Movement>();
                t.x += m.velX;
                t.y += m.velY;
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
                auto &a = e.get<Animation>();
                const auto &m = e.get<Movement>();

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

    void DrawingSystem::run(SDL_Renderer *ren)
    {
        static const bagel::Mask mask = bagel::MaskBuilder()
                                            .set<MTransform>()
                                            .set<Drawable>()
                                            .set<Animation>()
                                            .set<Movement>()
                                            .build();

        for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
        {
            if (e.test(mask))
            {
                const auto &t = e.get<MTransform>();
                const auto &a = e.get<Animation>();
                const auto &d = e.get<Drawable>();
                const auto &m = e.get<Movement>();

                int startFrame, frameCount;
                switch (a.state)
                {
                case Animation::RUN:
                    startFrame = d.runStart;
                    frameCount = d.runCount;
                    break;
                case Animation::JUMP:
                    startFrame = d.jumpStart;
                    frameCount = d.jumpCount;
                    break;
                default:
                    startFrame = d.idleStart;
                    frameCount = d.idleCount;
                    break;
                }

                const int frame = startFrame + (a.currentFrame % frameCount);
                SDL_FRect src = {frame * d.spriteW, 0.f, d.spriteW, d.spriteH};
                SDL_FRect dest = transformToFrect(t);

                const bool shouldFlip = m.facingLeft == d.defaultFacingLeft;
                const SDL_FlipMode flip = shouldFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
                SDL_RenderTextureRotated(ren, d.texture, &src, &dest, 0.0, nullptr, flip);
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
        static const bagel::Mask playerMask = bagel::MaskBuilder()
                                                  .set<Input>()
                                                  .set<MTransform>()
                                                  .build();

        static const bagel::Mask enemyMask = bagel::MaskBuilder()
                                                 .set<AI>()
                                                 .set<Movement>()
                                                 .set<MTransform>()
                                                 .build();

        float playerX = 0.f;
        float playerY = 0.f;

        for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
        {
            if (e.test(playerMask))
            {
                playerX = e.get<MTransform>().x;
                playerY = e.get<MTransform>().y;
                break;
            }
        }

        for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
        {
            if (e.test(enemyMask))
            {
                auto &ai = e.get<AI>();
                auto &t = e.get<MTransform>();
                auto &m = e.get<Movement>();

                const float distanceX = std::abs(t.x - playerX);
                const float distanceY = std::abs(t.y - playerY);

                if (distanceX < ai.detectionRange && distanceY < ai.detectionRange)
                {
                    ai.state = AI::CHASE_SHOOT;
                    m.velX = t.x < playerX ? ai.speed : -ai.speed;
                }

                else
                {
                    ai.state = AI::PATROL;
                    if (t.x < ai.patrolMinX)
                        m.velX = ai.speed;
                    else if (t.x > ai.patrolMaxX)
                        m.velX = -ai.speed;
                }

                if (m.velX < 0)
                    m.facingLeft = true;
                else if (m.velX > 0)
                    m.facingLeft = false;
            }
        }
    }
    void SoundSystem::run()
    {
    }
} // namespace megaman
