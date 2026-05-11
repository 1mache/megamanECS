#include "megaman.h"
#include "GlobalData.h"
#include "MTransform.h"
#include <cmath>
#include <iostream>
#include <vector>

namespace
{
constexpr float SPRITE_W    = 28.f;
constexpr float SPRITE_H    = 28.f;
constexpr int   RUN_START   = 0;
constexpr int   RUN_COUNT   = 4;
constexpr int   IDLE_START  = 8;
constexpr int   IDLE_COUNT  = 1;
constexpr int   JUMP_START  = 10;
constexpr int   JUMP_COUNT  = 1;
constexpr int   ANIM_SPEED  = 8;
constexpr float BULLET_SPEED = 0.15f;
} // namespace

namespace megaman
{
ent_type createPlayer(b2WorldId world, float x, float y, int hp)
{
    constexpr float halfW = SPRITE_W / (2 * GlobalData::PTM);
    constexpr float halfH = SPRITE_H / (2 * GlobalData::PTM);

    ent_type ent = bagel::World::createEntity();

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {x, y};
    bodyDef.fixedRotation = true;
    bodyDef.gravityScale = 0.f;
    bodyDef.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.id));
    b2BodyId body = b2CreateBody(world, &bodyDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.f;
    shapeDef.enableContactEvents = true;
    b2Polygon boxShape = b2MakeBox(halfW, halfH);
    b2CreatePolygonShape(body, &shapeDef, &boxShape);

    bagel::World::addComponent<Animation>(ent, {});
    bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(ent, {.mass = 1.f, .bodyId = body});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = static_cast<float>(hp)});
    bagel::World::addComponent<Input>(ent, {});
    bagel::World::addComponent<Weapon>(ent, {});

    return ent;
}

ent_type createPatroller(b2WorldId world, float x, float y, float hp, float patrolMinX, float patrolMaxX, float detectionRange, float speed)
{
    constexpr float halfW = 0.733f / 2.f;
    constexpr float halfH = 0.8f / 2.f;

    ent_type ent = bagel::World::createEntity();

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {x, y};
    bodyDef.fixedRotation = true;
    bodyDef.gravityScale = 0.f;
    bodyDef.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.id));
    b2BodyId body = b2CreateBody(world, &bodyDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.f;
    shapeDef.enableContactEvents = true;
    b2Polygon boxShape = b2MakeBox(halfW, halfH);
    b2CreatePolygonShape(body, &shapeDef, &boxShape);

    bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(ent, {.mass = 1, .velX = speed, .bodyId = body});
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

ent_type createLockster(b2WorldId world, float x, float y, float hp, float detectionRange, float chargeSpeed)
{
    constexpr float halfW = 0.7f / 2.f;
    constexpr float halfH = 0.7f / 2.f;

    ent_type ent = bagel::World::createEntity();

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {x, y};
    bodyDef.fixedRotation = true;
    bodyDef.gravityScale = 0.f;
    bodyDef.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.id));
    b2BodyId body = b2CreateBody(world, &bodyDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.f;
    shapeDef.enableContactEvents = true;
    b2Polygon boxShape = b2MakeBox(halfW, halfH);
    b2CreatePolygonShape(body, &shapeDef, &boxShape);

    bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(ent, {.mass = 1, .bodyId = body});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = hp});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<AI>(ent, {.type = AI::Type::Lockster,
                                         .detectionRange = detectionRange,
                                         .chargeSpeed = chargeSpeed,
                                         .spawnX = x});
    bagel::World::addComponent<Animation>(ent, {});

    return ent;
}

ent_type createBoss(float x, float y, float hp)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});
    bagel::World::addComponent<Movement>(ent, {.mass = 1});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = hp});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<AI>(ent, {});
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

ent_type createProjectile(float x, float y, float velX, float velY, bool fromEnemy)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<Drawable>(ent, {.texture = GlobalData::getShotTexture(), .spriteW = 16.f, .spriteH = 8.f, .drawScale = 1.f, .idleStart = 0, .idleCount = 1});
    bagel::World::addComponent<Animation>(ent, {});
    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y, .w = 0.2f, .h = 0.2f});
    bagel::World::addComponent<Movement>(ent, {.mass = 1, .velX = velX, .velY = velY});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Projectile>(ent, {.fromEnemy = fromEnemy});

    return ent;
}

ent_type createTrigger(float x, float y, float width, float height)
{
    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<MTransform>(ent, {.x = x, .y = y});
    bagel::World::addComponent<Collision>(ent, {.width = width, .height = height});

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

    bagel::World::addComponent<Drawable>(ent, {.texture = nullptr});
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
        bagel::MaskBuilder().set<Input>().set<Movement>().set<Weapon>().build();

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

            auto &w = e.get<Weapon>();
            if (w.shootCooldown > 0)
                --w.shootCooldown;

            if (b2Body_IsValid(m.bodyId))
            {
                constexpr float fps = static_cast<float>(GlobalData::FPS);
                b2Body_SetLinearVelocity(m.bodyId, {m.velX * fps, m.velY * fps});
            }

            if (keys[SDL_SCANCODE_SPACE])
            {
                if (w.shootCooldown <= 0)
                {
                    const auto &t = e.get<MTransform>();
                    const float velX = m.facingLeft ? -BULLET_SPEED : BULLET_SPEED;
                    ent_type bullet = createProjectile(t.x, t.y, velX, 0.f, false);
                    std::cout << "bullet created id=" << bullet.id << " tex=" << GlobalData::getShotTexture() << "\n";
                    w.shootCooldown = 20;
                }
            }
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
            if (b2Body_IsValid(m.bodyId))
                transformUpdateWithB2Pos(t, b2Body_GetPosition(m.bodyId));
            else
            {
                t.x += m.velX;
                t.y += m.velY;
            }
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

            if (!e.has<AI>() && !e.has<Projectile>())
            {
                const Animation::State newState =
                    m.velX != 0.f ? Animation::RUN : Animation::IDLE;

                if (newState != a.state)
                {
                    a.state = newState;
                    a.currentFrame = 0;
                    a.frameTimer = 0;
                }
            }

            if (a.state == Animation::IDLE)
            {
                a.currentFrame = 0;
                a.frameTimer = 0;
            }
            else if (++a.frameTimer >= ANIM_SPEED)
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

            if (d.texture == nullptr || frameCount <= 0)
                continue;

            const int frame = startFrame + (a.currentFrame % frameCount);
            SDL_FRect src = {frame * d.spriteW, 0.f, d.spriteW, d.spriteH};
            SDL_FRect dest = transformToFrect(t);

            const bool shouldFlip = m.facingLeft == d.defaultFacingLeft;
            const SDL_FlipMode flip = shouldFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            SDL_RenderTextureRotated(ren, d.texture, &src, &dest, 0.0, nullptr, flip);
        }
    }
}

void CollisionSystem::run(b2WorldId world)
{
    constexpr float dt       = 1.f / static_cast<float>(GlobalData::FPS);
    constexpr int   subSteps = 4;
    b2World_Step(world, dt, subSteps);

    static const bagel::Mask playerMask = bagel::MaskBuilder()
                                              .set<Input>()
                                              .set<MTransform>()
                                              .set<Health>()
                                              .build();
    static const bagel::Mask enemyMask = bagel::MaskBuilder()
                                             .set<Enemy>()
                                             .set<MTransform>()
                                             .set<Health>()
                                             .build();
    static const bagel::Mask bulletMask = bagel::MaskBuilder()
                                              .set<Projectile>()
                                              .set<MTransform>()
                                              .set<Movement>()
                                              .build();

    ent_type playerEnt{};
    bool playerFound = false;
    for (bagel::Entity p = bagel::Entity::first(); !p.eof(); p.next())
    {
        if (p.test(playerMask))
        {
            playerEnt = p.entity();
            playerFound = true;

            const auto &t = p.get<MTransform>();
            GlobalData::updateCamPosition(t.x, t.y);
            break;
        }
    }

    std::vector<ent_type> toDestroy;

    for (bagel::Entity bullet = bagel::Entity::first(); !bullet.eof(); bullet.next())
    {
        if (!bullet.test(bulletMask))
            continue;

        const MTransform bt = bullet.get<MTransform>();
        const bool isEnemyBullet = bullet.get<Projectile>().fromEnemy;

        if (isEnemyBullet)
        {
            if (playerFound)
            {
                bagel::Entity p{playerEnt};
                auto &ph = p.get<Health>();
                if (!ph.isInvulnerable)
                {
                    const MTransform pt = p.get<MTransform>();
                    const bool overlapX = std::abs(pt.x - bt.x) < (pt.w + bt.w) / 2.f;
                    const bool overlapY = std::abs(pt.y - bt.y) < (pt.h + bt.h) / 2.f;
                    if (overlapX && overlapY)
                    {
                        ph.points -= 0.5f;
                        ph.isInvulnerable = true;
                        ph.invulnerableTimer = 90;
                        toDestroy.push_back(bullet.entity());
                        std::cout << "enemy bullet hit player, hp=" << ph.points << "\n";
                    }
                }
            }
        }
        else
    {
            for (bagel::Entity en = bagel::Entity::first(); !en.eof(); en.next())
            {
                if (!en.test(enemyMask))
                    continue;
                const MTransform et = en.get<MTransform>();
                const bool overlapX = std::abs(et.x - bt.x) < (et.w + bt.w) / 2.f;
                const bool overlapY = std::abs(et.y - bt.y) < (et.h + bt.h) / 2.f;
                if (overlapX && overlapY)
                {
                    en.get<Health>().points -= 0.5f;
                    std::cout << "player bullet hit enemy id=" << en.entity().id << " hp=" << en.get<Health>().points << "\n";
                    toDestroy.push_back(bullet.entity());
                    break;
                }
            }
        }
    }

    for (ent_type e : toDestroy)
        bagel::Entity{e}.destroy();

    static const bagel::Mask inputMask    = bagel::MaskBuilder().set<Input>().set<Health>().build();
    static const bagel::Mask enemyTagMask = bagel::MaskBuilder().set<Enemy>().build();

    b2ContactEvents contacts = b2World_GetContactEvents(world);
    for (int i = 0; i < contacts.beginCount; ++i)
    {
        const b2ContactBeginTouchEvent &ev = contacts.beginEvents[i];
        b2BodyId bodyA = b2Shape_GetBody(ev.shapeIdA);
        b2BodyId bodyB = b2Shape_GetBody(ev.shapeIdB);

        int idA = static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyA)));
        int idB = static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyB)));

        bagel::Entity entA{ent_type{idA}};
        bagel::Entity entB{ent_type{idB}};

        bagel::Entity *pPlayer = nullptr;
        if (entA.test(inputMask) && entB.test(enemyTagMask))
            pPlayer = &entA;
        else if (entB.test(inputMask) && entA.test(enemyTagMask))
            pPlayer = &entB;

        if (pPlayer)
        {
            auto &ph = pPlayer->get<Health>();
            if (!ph.isInvulnerable)
            {
                ph.points -= 1.f;
                ph.isInvulnerable = true;
                ph.invulnerableTimer = 90;
                std::cout << "player hit by enemy, hp=" << ph.points << "\n";
            }
        }
    }
}

void HealthSystem::run()
{
    static const bagel::Mask mask = bagel::MaskBuilder().set<Health>().build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(mask))
        {
            auto &h = e.get<Health>();
            if (h.isInvulnerable && --h.invulnerableTimer <= 0)
                h.isInvulnerable = false;
            if (h.points <= 0)
                h.isDead = true;
        }
    }
}

namespace
{
    void updateFacing(Movement &m)
    {
        if (m.velX < 0)
            m.facingLeft = true;
        else if (m.velX > 0)
            m.facingLeft = false;
    }

    void tickPatroller(AI &ai, MTransform &t, Movement &m, float playerX, float playerY)
    {
        const float distX = std::abs(t.x - playerX);
        const float distY = std::abs(t.y - playerY);

        if (distX < ai.detectionRange && distY < ai.detectionRange)
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

        updateFacing(m);

        if (b2Body_IsValid(m.bodyId))
        {
            constexpr float fps = static_cast<float>(GlobalData::FPS);
            b2Body_SetLinearVelocity(m.bodyId, {m.velX * fps, 0.f});
        }

        if (ai.state == AI::CHASE_SHOOT)
        {
            if (ai.shootCooldown > 0)
                --ai.shootCooldown;
            else
            {
                const float velX = t.x < playerX ? BULLET_SPEED : -BULLET_SPEED;
                createProjectile(t.x, t.y, velX, 0.f, true);
                ai.shootCooldown = 60;
            }
        }
    }

    void tickLockster(AI &ai, MTransform &t, Movement &m, Animation &a, float playerX, float playerY)
    {
        const float distX = std::abs(t.x - playerX);
        const float distY = std::abs(t.y - playerY);

        if (distX < ai.detectionRange && distY < ai.detectionRange)
        {
            if (ai.alertTimer < 30)
            {
                ++ai.alertTimer;
                m.velX = 0.f;
                a.state = Animation::JUMP;

                if (ai.shotsFired < 3 && ai.alertTimer % 10 == 0)
                {
                    const float velX = t.x < playerX ? 0.15f : -0.15f;
                    createProjectile(t.x, t.y, velX, 0.f, true);
                    ++ai.shotsFired;
                }
            }
            else
            {
                m.velX = t.x < playerX ? ai.chargeSpeed : -ai.chargeSpeed;
                a.state = Animation::RUN;
            }
        }
        else
        {
            ai.alertTimer = 0;
            ai.shotsFired = 0;
            const float dx = ai.spawnX - t.x;
            if (std::abs(dx) > 0.05f)
                m.velX = dx > 0 ? ai.speed : -ai.speed;
            else
                m.velX = 0.f;
            a.state = Animation::IDLE;
        }

        updateFacing(m);

        if (b2Body_IsValid(m.bodyId))
        {
            constexpr float fps = static_cast<float>(GlobalData::FPS);
            b2Body_SetLinearVelocity(m.bodyId, {m.velX * fps, 0.f});
        }
    }
} // namespace

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
                                             .set<Animation>()
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
            auto &t  = e.get<MTransform>();
            auto &m  = e.get<Movement>();
            auto &a  = e.get<Animation>();

            switch (ai.type)
            {
            case AI::Type::Patroller:
                tickPatroller(ai, t, m, playerX, playerY);
                a.state = Animation::RUN;
                break;
            case AI::Type::Lockster:
                tickLockster(ai, t, m, a, playerX, playerY);
                break;
            }
        }
    }
}

void SoundSystem::run()
{
}
} // namespace megaman
