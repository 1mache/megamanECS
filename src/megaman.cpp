#include "megaman.h"
#include "GlobalData.h"
#include "MTransform.h"
#include <cmath>
#include <iostream>
#include <vector>

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
constexpr int   ANIM_SPEED = 8;
constexpr float BULLET_SPEED = 0.15f;
constexpr float PATROLLER_Y_RANGE = 1.5f;
constexpr bool  LOCKSTER_HAS_BULLETS = false;
constexpr float DAMAGEFROMBULLET = 0.5f;
constexpr float DAMAGEFROMENEMYCOLLISION = 1.f;
constexpr float EXPLOSION_SPRITE_W = 22.f;
constexpr float EXPLOSION_SPRITE_H = 24.f;
constexpr int   EXPLOSION_FRAMES = 3;
constexpr float EXPLOSION_SCALE = 2.f;
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
    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(ent, {.mass = 1.f, .bodyId = body});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = static_cast<float>(hp)});
    bagel::World::addComponent<DamageIntent>(ent, {});
    bagel::World::addComponent<Input>(ent, {});
    bagel::World::addComponent<Intent>(ent, {.speed = 0.05f});
    bagel::World::addComponent<Weapon>(ent, {});
    bagel::World::addComponent<Respawn>(
        ent,
        {.spawnX = x, .spawnY = y, .maxHp = static_cast<float>(hp)});

    return ent;
}

ent_type createPatroller(b2WorldId world,
                         float     x,
                         float     y,
                         float     hp,
                         float     patrolMinX,
                         float     patrolMaxX,
                         float     detectionRange,
                         float     speed)
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
    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(ent, {.mass = 1, .bodyId = body});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = hp});
    bagel::World::addComponent<DamageIntent>(ent, {});
    bagel::World::addComponent<Intent>(ent, {});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<AI>(ent,
                                   {.type = AI::Type::Patroller,
                                    .patrolMinX = patrolMinX,
                                    .patrolMaxX = patrolMaxX,
                                    .detectionRange = detectionRange,
                                    .speed = speed,
                                    .spawnX = x,
                                    .spawnY = y});
    bagel::World::addComponent<Weapon>(ent, {});
    bagel::World::addComponent<Animation>(ent, {});

    return ent;
}

ent_type createLockster(b2WorldId world,
                        float     x,
                        float     y,
                        float     hp,
                        float     detectionRange,
                        float     chargeSpeed)
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
    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(ent, {.mass = 1, .bodyId = body});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = hp});
    bagel::World::addComponent<DamageIntent>(ent, {});
    bagel::World::addComponent<Intent>(ent, {});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<AI>(ent,
                                   {.type = AI::Type::Lockster,
                                    .detectionRange = detectionRange,
                                    .chargeSpeed = chargeSpeed,
                                    .spawnX = x,
                                    .spawnY = y});
    bagel::World::addComponent<Weapon>(ent, {});
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

    bagel::World::addComponent<Drawable>(ent,
                                         {.texture = GlobalData::getShotTexture(),
                                          .spriteW = 16.f,
                                          .spriteH = 8.f,
                                          .drawScale = 1.f,
                                          .idleStart = 0,
                                          .idleCount = 1});
    bagel::World::addComponent<Animation>(ent, {});
    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = 0.2f, .h = 0.2f});
    bagel::World::addComponent<Movement>(ent,
                                         {.mass = 1, .velX = velX, .velY = velY});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Projectile>(ent, {.fromEnemy = fromEnemy});

    return ent;
}

ent_type createExplosion(float x, float y)
{
    constexpr float halfW = EXPLOSION_SPRITE_W / (2.f * GlobalData::PTM);
    constexpr float halfH = EXPLOSION_SPRITE_H / (2.f * GlobalData::PTM);

    ent_type ent = bagel::World::createEntity();

    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Drawable>(
        ent,
        {.texture = GlobalData::getExplosionTexture(),
         .spriteW = EXPLOSION_SPRITE_W,
         .spriteH = EXPLOSION_SPRITE_H,
         .drawScale = EXPLOSION_SCALE,
         .idleStart = 0,
         .idleCount = EXPLOSION_FRAMES});
    bagel::World::addComponent<Animation>(ent, {});
    bagel::World::addComponent<Movement>(ent, {});
    bagel::World::addComponent<Explosion>(ent, {});

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

// ============= SYSTEMS =============

void inputSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Input>().set<Intent>().build();

    SDL_PumpEvents();
    const bool* keys = SDL_GetKeyboardState(nullptr);

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(mask))
        {
            auto& intent = e.get<Intent>();
            intent.moveLeft = keys[SDL_SCANCODE_LEFT];
            intent.moveRight = keys[SDL_SCANCODE_RIGHT];
            intent.moveUp = keys[SDL_SCANCODE_UP];
            intent.moveDown = keys[SDL_SCANCODE_DOWN];
            intent.shoot = keys[SDL_SCANCODE_SPACE];
        }
    }
}

void movementSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<MTransform>().set<Movement>().build();

    constexpr float fps = static_cast<float>(GlobalData::FPS);
    constexpr float velY = 0.083f;

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        auto& t = e.get<MTransform>();
        auto& m = e.get<Movement>();

        if (e.has<Intent>())
        {
            const auto& intent = e.get<Intent>();
            m.velX = intent.moveLeft    ? -intent.speed
                     : intent.moveRight ? intent.speed
                                        : 0.f;
            m.velY = intent.moveUp ? velY : intent.moveDown ? -velY : 0.f;

            if (intent.moveLeft)
                m.facingLeft = true;
            else if (intent.moveRight)
                m.facingLeft = false;

            if (b2Body_IsValid(m.bodyId))
                b2Body_SetLinearVelocity(m.bodyId, {m.velX * fps, m.velY * fps});
        }

        if (b2Body_IsValid(m.bodyId))
            transformUpdateWithB2Pos(t, b2Body_GetPosition(m.bodyId));
        else
        {
            t.x += m.velX;
            t.y += m.velY;
        }
    }
}

void shootingSystem()
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<Intent>()
                                        .set<Weapon>()
                                        .set<MTransform>()
                                        .set<Movement>()
                                        .build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        auto& w = e.get<Weapon>();
        if (w.shootCooldown > 0)
            --w.shootCooldown;

        const auto& intent = e.get<Intent>();
        if (intent.shoot && w.shootCooldown <= 0)
        {
            const auto& t = e.get<MTransform>();
            const bool  fromEnemy = !e.has<Input>();
            const float bVelX =
                e.get<Movement>().facingLeft ? -BULLET_SPEED : BULLET_SPEED;
            ent_type bullet = createProjectile(t.x, t.y, bVelX, 0.f, fromEnemy);
            if (!fromEnemy)
                std::cout << "bullet created id=" << bullet.id
                          << " tex=" << GlobalData::getShotTexture() << "\n";
            w.shootCooldown = fromEnemy ? 60 : 20;
        }
    }
}

void animationSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Animation>().set<Movement>().build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(mask))
        {
            if (e.has<Explosion>())
                continue;

            auto&       a = e.get<Animation>();
            const auto& m = e.get<Movement>();

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

void drawSystem(SDL_Renderer* ren)
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
            const auto& t = e.get<MTransform>();
            const auto& a = e.get<Animation>();
            const auto& d = e.get<Drawable>();
            const auto& m = e.get<Movement>();

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

            if (e.has<Health>())
            {
                const auto& h = e.get<Health>();
                if (h.isInvulnerable && (h.invulnerableTimer / 4) % 2 == 0)
                    continue;
            }

            const int frame = startFrame + (a.currentFrame % frameCount);
            SDL_FRect src = {frame * d.spriteW, 0.f, d.spriteW, d.spriteH};
            SDL_FRect dest = transformToFrect(t);

            const bool         shouldFlip = m.facingLeft == d.defaultFacingLeft;
            const SDL_FlipMode flip =
                shouldFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            SDL_RenderTextureRotated(ren,
                                     d.texture,
                                     &src,
                                     &dest,
                                     0.0,
                                     nullptr,
                                     flip);
        }
    }
}

void collisionSystem(b2WorldId world)
{
    constexpr float dt = 1.f / static_cast<float>(GlobalData::FPS);
    constexpr int   subSteps = 4;
    b2World_Step(world, dt, subSteps);

    static const bagel::Mask playerMask = bagel::MaskBuilder()
                                              .set<Input>()
                                              .set<MTransform>()
                                              .set<Health>()
                                              .set<DamageIntent>()
                                              .build();
    static const bagel::Mask enemyMask = bagel::MaskBuilder()
                                             .set<Enemy>()
                                             .set<MTransform>()
                                             .set<Health>()
                                             .set<DamageIntent>()
                                             .build();
    static const bagel::Mask bulletMask = bagel::MaskBuilder()
                                              .set<Projectile>()
                                              .set<MTransform>()
                                              .set<Movement>()
                                              .build();

    ent_type playerEnt{};
    bool     playerFound = false;
    for (bagel::Entity p = bagel::Entity::first(); !p.eof(); p.next())
    {
        if (p.test(playerMask))
        {
            playerEnt = p.entity();
            playerFound = true;

            const auto& t = p.get<MTransform>();
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

        if (bullet.get<Projectile>().fromEnemy)
        {
            if (playerFound)
            {
                bagel::Entity p{playerEnt};
                const auto&   ph = p.get<Health>();
                if (!ph.isInvulnerable)
                {
                    const MTransform pt = p.get<MTransform>();
                    const bool overlapX = std::abs(pt.x - bt.x) < (pt.w + bt.w);
                    const bool overlapY = std::abs(pt.y - bt.y) < (pt.h + bt.h);
                    if (overlapX && overlapY)
                    {
                        p.get<DamageIntent>() = {.amount = DAMAGEFROMBULLET,
                                                 .pending = true,
                                                 .fromContact = false};
                        toDestroy.push_back(bullet.entity());
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
                const bool       overlapX = std::abs(et.x - bt.x) < (et.w + bt.w);
                const bool       overlapY = std::abs(et.y - bt.y) < (et.h + bt.h);
                if (overlapX && overlapY)
                {
                    en.get<DamageIntent>() = {.amount = DAMAGEFROMBULLET,
                                              .pending = true,
                                              .fromContact = false};
                    toDestroy.push_back(bullet.entity());
                    break;
                }
            }
        }
    }

    for (ent_type e : toDestroy)
        bagel::Entity{e}.destroy();

    static const bagel::Mask inputMask =
        bagel::MaskBuilder().set<Input>().set<DamageIntent>().build();
    static const bagel::Mask enemyTagMask =
        bagel::MaskBuilder().set<Enemy>().build();

    b2ContactEvents contacts = b2World_GetContactEvents(world);
    for (int i = 0; i < contacts.beginCount; ++i)
    {
        const b2ContactBeginTouchEvent& ev = contacts.beginEvents[i];
        b2BodyId                        bodyA = b2Shape_GetBody(ev.shapeIdA);
        b2BodyId                        bodyB = b2Shape_GetBody(ev.shapeIdB);

        const int idA =
            static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyA)));
        const int idB =
            static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyB)));

        bagel::Entity entA{ent_type{idA}};
        bagel::Entity entB{ent_type{idB}};

        bagel::Entity* pPlayer = nullptr;
        if (entA.test(inputMask) && entB.test(enemyTagMask))
            pPlayer = &entA;
        else if (entB.test(inputMask) && entA.test(enemyTagMask))
            pPlayer = &entB;

        if (pPlayer)
            pPlayer->get<DamageIntent>() = {.amount = DAMAGEFROMENEMYCOLLISION,
                                            .pending = true,
                                            .fromContact = true};
    }

    if (playerFound)
    {
        bagel::Entity    player{playerEnt};
        const MTransform pt = player.get<MTransform>();
        for (bagel::Entity enemy = bagel::Entity::first(); !enemy.eof();
             enemy.next())
        {
            if (!enemy.test(enemyMask))
                continue;

            const MTransform et = enemy.get<MTransform>();
            const bool       overlapX = std::abs(pt.x - et.x) < (pt.w + et.w);
            const bool       overlapY = std::abs(pt.y - et.y) < (pt.h + et.h);
            if (overlapX && overlapY)
                player.get<DamageIntent>() = {.amount = DAMAGEFROMENEMYCOLLISION,
                                              .pending = true,
                                              .fromContact = true};
        }
    }
}

void damageSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<DamageIntent>().set<Health>().build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        auto& di = e.get<DamageIntent>();
        if (!di.pending)
            continue;

        auto& h = e.get<Health>();
        if (di.fromContact)
        {
            if (h.isContactInvulnerable)
            {
                di.pending = false;
                continue;
            }
            h.isContactInvulnerable = true;
        }
        if (!h.isInvulnerable)
        {
            h.points -= di.amount;
            h.isInvulnerable = true;
            h.invulnerableTimer = 90;
            h.justHit = true;
            if (di.fromContact)
                std::cout << "player hit by enemy, hp=" << h.points << "\n";
            else
                std::cout << (e.has<Input>() ? "enemy bullet hit player"
                                             : "player bullet hit enemy")
                          << ", hp=" << h.points << "\n";

            if (e.has<Input>())
            {
                static const bagel::Mask aiMask =
                    bagel::MaskBuilder().set<AI>().build();
                for (bagel::Entity en = bagel::Entity::first(); !en.eof(); en.next())
                {
                    if (en.test(aiMask))
                        en.get<AI>().freezeFrames = 60;
                }
            }
        }
        di.pending = false;
    }
}

void healthSystem()
{
    static const bagel::Mask mask = bagel::MaskBuilder().set<Health>().build();

    std::vector<ent_type> toDestroy;

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        auto& h = e.get<Health>();
        if (h.isInvulnerable && --h.invulnerableTimer <= 0)
        {
            h.isInvulnerable = false;
            h.isContactInvulnerable = false;
        }
        if (h.points <= 0)
        {
            if (e.has<Enemy>())
            {
                const MTransform& t = e.get<MTransform>();
                createExplosion(t.x, t.y);
                if (e.has<Movement>())
                {
                    Movement& m = e.get<Movement>();
                    if (b2Body_IsValid(m.bodyId))
                        b2DestroyBody(m.bodyId);
                    m.bodyId = b2_nullBodyId;
                }
                toDestroy.push_back(e.entity());
            }
            else
            {
                h.isDead = true;
            }
        }
    }

    for (ent_type e : toDestroy)
        bagel::Entity{e}.destroy();
}

namespace
{
void tickPatroller(AI&               ai,
                   const MTransform& t,
                   Intent&           intent,
                   float             playerX,
                   float             playerY)
{
    const float distX = std::abs(t.x - playerX);
    const float distY = std::abs(t.y - playerY);

    intent = {};
    intent.speed = ai.speed;

    if (distX < ai.detectionRange && distY < PATROLLER_Y_RANGE)
    {
        ai.state = AI::CHASE_SHOOT;
        if (t.x < playerX)
            intent.moveRight = true;
        else
            intent.moveLeft = true;
        intent.shoot = true;
    }
    else
    {
        ai.state = AI::PATROL;
        if (t.x <= ai.patrolMinX)
            ai.patrollingRight = true;
        else if (t.x >= ai.patrolMaxX)
            ai.patrollingRight = false;
        if (ai.patrollingRight)
            intent.moveRight = true;
        else
            intent.moveLeft = true;
    }
}

void tickLockster(AI&               ai,
                  const MTransform& t,
                  Movement&         m,
                  Intent&           intent,
                  Animation&        a,
                  float             playerX,
                  float             playerY)
{
    constexpr float Y_MARGIN = 1.5f;
    const float     distX = std::abs(t.x - playerX);
    const float     distY = std::abs(t.y - playerY);

    intent = {};

    const bool inRange = distX < ai.detectionRange && distY < Y_MARGIN;
    const bool isCharging = ai.alertTimer >= 30;

    if (isCharging)
    {
        const bool reached = std::abs(t.x - ai.targetX) < 0.2f;
        if (reached)
        {
            ai.alertTimer = 0;
            ai.shotsFired = 0;
        }
        else
        {
            intent.speed = ai.chargeSpeed;
            if (t.x < ai.targetX)
                intent.moveRight = true;
            else
                intent.moveLeft = true;
            a.state = Animation::RUN;
        }
    }
    else if (inRange)
    {
        ++ai.alertTimer;
        a.state = Animation::JUMP;

        if (ai.alertTimer == 30)
            ai.targetX = playerX;

        if constexpr (LOCKSTER_HAS_BULLETS)
        {
            if (ai.shotsFired < 3 && ai.alertTimer % 10 == 0)
            {
                const float velX = t.x < playerX ? 0.15f : -0.15f;
                createProjectile(t.x, t.y, velX, 0.f, true);
                ++ai.shotsFired;
            }
        }
    }
    else
    {
        ai.alertTimer = 0;
        ai.shotsFired = 0;
        a.state = Animation::IDLE;
        a.currentFrame = 0;
        a.frameTimer = 0;
    }

    if (b2Body_IsValid(m.bodyId))
    {
        const b2Vec2 pos = b2Body_GetPosition(m.bodyId);
        b2Body_SetTransform(m.bodyId,
                            {pos.x, ai.spawnY},
                            b2Body_GetRotation(m.bodyId));
    }
}
} // namespace

void aiSystem()
{
    static const bagel::Mask playerMask =
        bagel::MaskBuilder().set<Input>().set<MTransform>().build();

    static const bagel::Mask enemyMask = bagel::MaskBuilder()
                                             .set<AI>()
                                             .set<Movement>()
                                             .set<Intent>()
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
            auto& ai = e.get<AI>();
            auto& intent = e.get<Intent>();

            if (ai.freezeFrames > 0)
            {
                --ai.freezeFrames;
                const float spd = intent.speed;
                intent = {};
                intent.speed = spd;
                continue;
            }

            const auto& t = e.get<MTransform>();
            auto&       m = e.get<Movement>();
            auto&       a = e.get<Animation>();

            switch (ai.type)
            {
            case AI::Type::Patroller:
                tickPatroller(ai, t, intent, playerX, playerY);
                a.state = Animation::RUN;
                break;
            case AI::Type::Lockster:
                tickLockster(ai, t, m, intent, a, playerX, playerY);
                break;
            }
        }
    }
}

void respawnSystem()
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<Respawn>()
                                        .set<Health>()
                                        .set<Movement>()
                                        .set<MTransform>()
                                        .build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        auto& r = e.get<Respawn>();
        auto& h = e.get<Health>();
        auto& m = e.get<Movement>();
        auto& t = e.get<MTransform>();

        auto zeroIntent = [&]() {
            if (e.has<Intent>())
            {
                auto&       intent = e.get<Intent>();
                const float spd = intent.speed;
                intent = {};
                intent.speed = spd;
            }
        };

        if (!r.isRespawning && h.justHit)
        {
            h.justHit = false;
            r.isRespawning = true;
            r.flickerTimer = 60;
            if (b2Body_IsValid(m.bodyId))
                b2Body_SetLinearVelocity(m.bodyId, {0.f, 0.f});
            zeroIntent();

            static const bagel::Mask bulletMask =
                bagel::MaskBuilder().set<Projectile>().build();
            for (bagel::Entity b = bagel::Entity::first(); !b.eof(); b.next())
            {
                if (b.test(bulletMask))
                    b.destroy();
            }
        }
        else if (r.isRespawning)
        {
            if (--r.flickerTimer <= 0)
            {
                if (b2Body_IsValid(m.bodyId))
                {
                    b2Body_SetTransform(m.bodyId,
                                        {r.spawnX, r.spawnY},
                                        b2Body_GetRotation(m.bodyId));
                    transformUpdateWithB2Pos(t, b2Body_GetPosition(m.bodyId));
                    b2Body_SetLinearVelocity(m.bodyId, {0.f, 0.f});
                }
                h.isInvulnerable = true;
                h.invulnerableTimer = 60;
                r.isRespawning = false;

                static const bagel::Mask aiMask = bagel::MaskBuilder()
                                                      .set<AI>()
                                                      .set<Movement>()
                                                      .set<MTransform>()
                                                      .build();
                for (bagel::Entity en = bagel::Entity::first(); !en.eof(); en.next())
                {
                    if (!en.test(aiMask))
                        continue;
                    auto& eai = en.get<AI>();
                    auto& em = en.get<Movement>();
                    auto& et = en.get<MTransform>();
                    if (b2Body_IsValid(em.bodyId))
                    {
                        b2Body_SetTransform(em.bodyId,
                                            {eai.spawnX, eai.spawnY},
                                            b2Body_GetRotation(em.bodyId));
                        transformUpdateWithB2Pos(et, b2Body_GetPosition(em.bodyId));
                        b2Body_SetLinearVelocity(em.bodyId, {0.f, 0.f});
                    }
                    eai.alertTimer = 0;
                    eai.shotsFired = 0;
                    eai.freezeFrames = 0;
                    eai.patrollingRight = true;
                }
            }
            else
            {
                if (b2Body_IsValid(m.bodyId))
                    b2Body_SetLinearVelocity(m.bodyId, {0.f, 0.f});
                zeroIntent();
            }
        }
    }
}

void explosionSystem()
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<Explosion>()
                                        .set<Animation>()
                                        .set<Drawable>()
                                        .build();

    std::vector<ent_type> toDestroy;

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        auto&       a = e.get<Animation>();
        const auto& d = e.get<Drawable>();

        if (++a.frameTimer >= ANIM_SPEED)
        {
            a.frameTimer = 0;
            ++a.currentFrame;
            if (a.currentFrame >= d.idleCount)
                toDestroy.push_back(e.entity());
        }
    }

    for (ent_type e : toDestroy)
        bagel::Entity{e}.destroy();
}
} // namespace megaman
