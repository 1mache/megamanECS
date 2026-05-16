#include "megaman.h"
#include "GlobalData.h"
#include "MTransform.h"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

namespace
{
// --- Player ---
constexpr float PLAYER_SPRITE_W   = 28.f;
constexpr float PLAYER_SPRITE_H   = 28.f;
constexpr int   PLAYER_IDLE_START = 8;
constexpr int   PLAYER_IDLE_COUNT = 1;
constexpr int   PLAYER_RUN_START  = 0;
constexpr int   PLAYER_RUN_COUNT  = 4;
constexpr int   PLAYER_JUMP_START = 10;
constexpr int   PLAYER_JUMP_COUNT = 1;

// --- Patroller enemy ---
constexpr float PATROLLER_SPRITE_W   = 24.f;
constexpr float PATROLLER_SPRITE_H   = 24.f;
constexpr int   PATROLLER_IDLE_START = 0;
constexpr int   PATROLLER_IDLE_COUNT = 1;
constexpr int   PATROLLER_RUN_START  = 0;
constexpr int   PATROLLER_RUN_COUNT  = 2;
constexpr int   PATROLLER_JUMP_START = 0;
constexpr int   PATROLLER_JUMP_COUNT = 1;
constexpr float PATROLLER_Y_RANGE    = 1.5f;

// --- Lockster enemy ---
constexpr float LOCKSTER_SPRITE_W     = 24.f;
constexpr float LOCKSTER_SPRITE_H     = 24.f;
constexpr int   LOCKSTER_IDLE_START   = 0;
constexpr int   LOCKSTER_IDLE_COUNT   = 2;
constexpr int   LOCKSTER_ALERT_START  = 2;
constexpr int   LOCKSTER_ALERT_COUNT  = 4;
constexpr int   LOCKSTER_CHARGE_START = 6;
constexpr int   LOCKSTER_CHARGE_COUNT = 2;
constexpr bool  LOCKSTER_HAS_BULLETS  = false;

// --- Projectile ---
constexpr float BULLET_SPEED  = 0.5f;
constexpr float SHOT_SPRITE_W = 8.f;
constexpr float SHOT_SPRITE_H = 8.f;
constexpr float BULLET_HALF_W = 0.2f;
constexpr float BULLET_HALF_H = 0.1f;


// --- Explosion ---
constexpr float EXPLOSION_SPRITE_W = 22.f;
constexpr float EXPLOSION_SPRITE_H = 24.f;
constexpr int   EXPLOSION_FRAMES   = 3;

// --- Damage ---
constexpr float BULLET_DAMAGE        = 0.5f;
constexpr float ENEMY_CONTACT_DAMAGE = 1.f;

template <typename AnimT>
void tickAnim(AnimT& anim, megaman::RenderFrame& rf)
{
    const std::size_t             clipIdx = static_cast<std::size_t>(anim.state);
    const megaman::AnimationClip& clip    = anim.clips[clipIdx];

    if (anim.state != anim.prev)
    {
        anim.frame = 0;
        anim.timer = 0;
        anim.prev  = anim.state;
    }

    rf.finishedThisTick = false;

    if (clip.frameCount > 1)
    {
        if (++anim.timer >= clip.framesPerStep)
        {
            anim.timer = 0;
            ++anim.frame;
            if (anim.frame >= clip.frameCount)
            {
                if (clip.loop)
                    anim.frame = 0;
                else
                {
                    anim.frame          = clip.frameCount - 1;
                    rf.finishedThisTick = true;
                }
            }
        }
    }

    rf.spriteIndex = clip.startFrame + anim.frame;
}
} // namespace

namespace megaman
{
ent_type createPlayer(b2WorldId world, float x, float y, int hp, SDL_Texture* tex)
{
    constexpr float halfW = PLAYER_SPRITE_W / (2 * GlobalData::PTM);
    constexpr float halfH = PLAYER_SPRITE_H / (2 * GlobalData::PTM);

    ent_type ent = bagel::World::createEntity();

    b2BodyDef bodyDef     = b2DefaultBodyDef();
    bodyDef.type          = b2_dynamicBody;
    bodyDef.position      = {x, y};
    bodyDef.fixedRotation = true;
    bodyDef.userData      = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.id));
    b2BodyId body         = b2CreateBody(world, &bodyDef);

    b2ShapeDef shapeDef                 = b2DefaultShapeDef();
    shapeDef.density                    = 1.f;
    shapeDef.material.friction          = 0.f;
    shapeDef.material.restitution       = 0.f;
    shapeDef.material.rollingResistance = 0.f;
    shapeDef.material.tangentSpeed      = 0.f;
    shapeDef.enableContactEvents        = true;
    shapeDef.filter.categoryBits        = CAT_PLAYER;
    shapeDef.filter.maskBits            = CAT_WORLD | CAT_ENEMY | CAT_ENEMY_BULLET;
    b2Polygon boxShape                  = b2MakeBox(halfW, halfH);
    b2CreatePolygonShape(body, &shapeDef, &boxShape);

    bagel::World::addComponent<PlayerAnimation>(
        ent,
        {.clips = {AnimationClip{PLAYER_IDLE_START, PLAYER_IDLE_COUNT},   // [0] Idle
                   AnimationClip{PLAYER_RUN_START,  PLAYER_RUN_COUNT},   // [1] Run
                   AnimationClip{PLAYER_JUMP_START, PLAYER_JUMP_COUNT}}}); // [2] Jump
    bagel::World::addComponent<RenderFrame>(ent, {});
    bagel::World::addComponent<Drawable>(
        ent,
        {.texture = tex, .spriteW = PLAYER_SPRITE_W, .spriteH = PLAYER_SPRITE_H});
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
        {.spawnX            = x,
         .spawnY            = y,
         .lastCheckpointX   = x,
         .lastCheckpointY   = y,
         .maxHp             = static_cast<float>(hp)});

    return ent;
}

ent_type createPatroller(b2WorldId    world,
                         float        x,
                         float        y,
                         float        hp,
                         float        patrolMinX,
                         float        patrolMaxX,
                         float        detectionRange,
                         float        speed,
                         SDL_Texture* tex)
{
    constexpr float halfW = PATROLLER_SPRITE_W / (2 * GlobalData::PTM);
    constexpr float halfH = PATROLLER_SPRITE_H / (2 * GlobalData::PTM);

    ent_type ent = bagel::World::createEntity();

    b2BodyDef bodyDef     = b2DefaultBodyDef();
    bodyDef.type          = b2_dynamicBody;
    bodyDef.position      = {x, y};
    bodyDef.fixedRotation = true;
    bodyDef.gravityScale  = 0.f;
    bodyDef.userData      = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.id));
    b2BodyId body         = b2CreateBody(world, &bodyDef);

    b2ShapeDef shapeDef          = b2DefaultShapeDef();
    shapeDef.density             = 1.f;
    shapeDef.enableContactEvents = true;
    shapeDef.filter.categoryBits = CAT_ENEMY;
    shapeDef.filter.maskBits     = CAT_WORLD | CAT_PLAYER | CAT_PLAYER_BULLET;
    b2Polygon boxShape           = b2MakeBox(halfW, halfH);
    b2CreatePolygonShape(body, &shapeDef, &boxShape);

    bagel::World::addComponent<PatrollerAnimation>(
        ent,
        {.clips = {
             AnimationClip{PATROLLER_RUN_START, PATROLLER_RUN_COUNT}}}); // [0] Run
    bagel::World::addComponent<RenderFrame>(ent, {});
    bagel::World::addComponent<Drawable>(ent,
                                         {.texture           = tex,
                                          .spriteW           = PATROLLER_SPRITE_W,
                                          .spriteH           = PATROLLER_SPRITE_H,
                                          .defaultFacingLeft = false});
    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(ent, {.mass = 1, .bodyId = body});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = hp});
    bagel::World::addComponent<DamageIntent>(ent, {});
    bagel::World::addComponent<Intent>(ent, {});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<AI>(ent,
                                   {.type           = AI::Type::Patroller,
                                    .patrolMinX     = patrolMinX,
                                    .patrolMaxX     = patrolMaxX,
                                    .detectionRange = detectionRange,
                                    .speed          = speed,
                                    .spawnX         = x,
                                    .spawnY         = y});
    bagel::World::addComponent<Weapon>(ent, {});

    return ent;
}

ent_type createLockster(b2WorldId    world,
                        float        x,
                        float        y,
                        float        hp,
                        float        detectionRange,
                        float        chargeSpeed,
                        SDL_Texture* tex)
{
    constexpr float halfW = LOCKSTER_SPRITE_W / (2 * GlobalData::PTM);
    constexpr float halfH = LOCKSTER_SPRITE_H / (2 * GlobalData::PTM);

    ent_type ent = bagel::World::createEntity();

    b2BodyDef bodyDef     = b2DefaultBodyDef();
    bodyDef.type          = b2_dynamicBody;
    bodyDef.position      = {x, y};
    bodyDef.fixedRotation = true;
    bodyDef.gravityScale  = 0.f;
    bodyDef.userData      = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.id));
    b2BodyId body         = b2CreateBody(world, &bodyDef);

    b2ShapeDef shapeDef          = b2DefaultShapeDef();
    shapeDef.density             = 1.f;
    shapeDef.enableContactEvents = true;
    shapeDef.filter.categoryBits = CAT_ENEMY;
    shapeDef.filter.maskBits     = CAT_WORLD | CAT_PLAYER | CAT_PLAYER_BULLET;
    b2Polygon boxShape           = b2MakeBox(halfW, halfH);
    b2CreatePolygonShape(body, &shapeDef, &boxShape);

    bagel::World::addComponent<LocksterAnimation>(
        ent,
        {.clips = {AnimationClip{LOCKSTER_IDLE_START, 1}, // [0] Idle — frozen
                   AnimationClip{LOCKSTER_CHARGE_START,
                                 LOCKSTER_CHARGE_COUNT}, // [1] Charge
                   AnimationClip{LOCKSTER_ALERT_START,
                                 LOCKSTER_ALERT_COUNT}}}); // [2] Alert
    bagel::World::addComponent<RenderFrame>(ent, {});
    bagel::World::addComponent<Drawable>(ent,
                                         {.texture           = tex,
                                          .spriteW           = LOCKSTER_SPRITE_W,
                                          .spriteH           = LOCKSTER_SPRITE_H,
                                          .defaultFacingLeft = false});
    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(ent, {.mass = 1, .bodyId = body});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = hp});
    bagel::World::addComponent<DamageIntent>(ent, {});
    bagel::World::addComponent<Intent>(ent, {});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<AI>(ent,
                                   {.type           = AI::Type::Lockster,
                                    .detectionRange = detectionRange,
                                    .chargeSpeed    = chargeSpeed,
                                    .spawnX         = x,
                                    .spawnY         = y});
    bagel::World::addComponent<Weapon>(ent, {});

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

ent_type createProjectile(b2WorldId world,
                          float     x,
                          float     y,
                          float     velX,
                          float     velY,
                          bool      fromEnemy)
{
    constexpr float fps = static_cast<float>(GlobalData::FPS);

    ent_type ent = bagel::World::createEntity();

    b2BodyDef bodyDef     = b2DefaultBodyDef();
    bodyDef.type          = b2_dynamicBody;
    bodyDef.position      = {x, y};
    bodyDef.fixedRotation = true;
    bodyDef.gravityScale  = 0.f;
    bodyDef.isBullet      = true;
    bodyDef.userData      = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.id));
    b2BodyId body         = b2CreateBody(world, &bodyDef);
    b2Body_SetLinearVelocity(body, {velX * fps, velY * fps});

    b2ShapeDef shapeDef          = b2DefaultShapeDef();
    shapeDef.density             = 1.f;
    shapeDef.enableContactEvents = true;
    shapeDef.filter.categoryBits = fromEnemy ? CAT_ENEMY_BULLET : CAT_PLAYER_BULLET;
    shapeDef.filter.maskBits =
        fromEnemy ? (CAT_WORLD | CAT_PLAYER) : (CAT_WORLD | CAT_ENEMY);
    b2Polygon boxShape = b2MakeBox(BULLET_HALF_W, BULLET_HALF_H);
    b2CreatePolygonShape(body, &shapeDef, &boxShape);

    bagel::World::addComponent<Drawable>(ent,
                                         {.texture = GlobalData::getShotTexture(),
                                          .spriteW = SHOT_SPRITE_W,
                                          .spriteH = SHOT_SPRITE_H});
    bagel::World::addComponent<RenderFrame>(ent, {});
    bagel::World::addComponent<MTransform>(
        ent,
        {.x = x, .y = y, .w = BULLET_HALF_W, .h = BULLET_HALF_H});
    bagel::World::addComponent<Movement>(
        ent,
        {.mass = 1, .velX = velX, .velY = velY, .bodyId = body});
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
         .spriteH = EXPLOSION_SPRITE_H});
    bagel::World::addComponent<ExplosionAnimation>(
        ent,
        {.clips = {AnimationClip{0,
                                 EXPLOSION_FRAMES,
                                 ANIM_SPEED,
                                 false}}}); // [0] Playing, one-shot
    bagel::World::addComponent<RenderFrame>(ent, {});
    bagel::World::addComponent<Movement>(ent, {});

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
            auto& intent     = e.get<Intent>();
            intent.moveLeft  = keys[SDL_SCANCODE_LEFT];
            intent.moveRight = keys[SDL_SCANCODE_RIGHT];
            intent.moveUp    = keys[SDL_SCANCODE_UP];
            intent.moveDown  = keys[SDL_SCANCODE_DOWN];
            intent.shoot     = keys[SDL_SCANCODE_SPACE];
        }
    }
}

void movementSystem(float sceneMinY)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<MTransform>().set<Movement>().build();

    constexpr float fps  = static_cast<float>(GlobalData::FPS);
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
            if (intent.moveLeft)
                m.facingLeft = true;
            else if (intent.moveRight)
                m.facingLeft = false;

            m.velX = intent.moveLeft    ? -intent.speed
                     : intent.moveRight ? intent.speed
                                        : 0.f;

            if (intent.moveUp)
                b2Body_ApplyLinearImpulse(m.bodyId, {0.f, 10.f}, {t.x, t.y}, true);
            auto currentVel = b2Body_GetLinearVelocity(m.bodyId);


            if (b2Body_IsValid(m.bodyId))
                b2Body_SetLinearVelocity(m.bodyId, {m.velX * fps, currentVel.y});
        }

        if (b2Body_IsValid(m.bodyId))
            transformUpdateWithB2Pos(t, b2Body_GetPosition(m.bodyId));

        if (e.has<Input>() && e.has<DamageIntent>() && e.has<Health>())
        {
            auto& di = e.get<DamageIntent>();
            auto& h  = e.get<Health>();
            if (t.y < sceneMinY - FALL_KILL_MARGIN_M && !di.pending && !h.isInvulnerable)
                di = {.amount = FALL_DAMAGE_HP, .pending = true, .fromContact = false, .fromFall = true};
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
            const auto& t         = e.get<MTransform>();
            bool        fromEnemy = !e.has<Input>();
            float       bVelX =
                e.get<Movement>().facingLeft ? -BULLET_SPEED : BULLET_SPEED;
            if (fromEnemy)
                bVelX *= 0.5f; // enemy bullets are slower for balance

            ent_type bullet = createProjectile(GlobalData::getBoxWorld(),
                                               t.x,
                                               t.y,
                                               bVelX,
                                               0.f,
                                               fromEnemy);
            if (!fromEnemy)
                std::cout << "bullet created id=" << bullet.id
                          << " tex=" << GlobalData::getShotTexture() << "\n";
            w.shootCooldown = fromEnemy ? 60 : 20;
        }
    }
}

void playerAnimSystem()
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<PlayerAnimation>()
                                        .set<Movement>()
                                        .set<RenderFrame>()
                                        .build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        auto&       a  = e.get<PlayerAnimation>();
        const auto& m  = e.get<Movement>();
        auto&       rf = e.get<RenderFrame>();

        constexpr float JUMP_VEL_EPSILON = 0.1f;
        const auto      vel              = b2Body_GetLinearVelocity(m.bodyId);
        if (std::fabs(vel.y) > JUMP_VEL_EPSILON)
            a.state = PlayerAnimation::State::Jump;
        else if (m.velX != 0.f)
            a.state = PlayerAnimation::State::Run;
        else
            a.state = PlayerAnimation::State::Idle;
        tickAnim(a, rf);
    }
}

void patrollerAnimSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<PatrollerAnimation>().set<RenderFrame>().build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;
        tickAnim(e.get<PatrollerAnimation>(), e.get<RenderFrame>());
    }
}

void locksterAnimSystem()
{
    static const bagel::Mask locksterMask = bagel::MaskBuilder()
                                                .set<LocksterAnimation>()
                                                .set<AI>()
                                                .set<MTransform>()
                                                .set<RenderFrame>()
                                                .build();
    static const bagel::Mask playerMask =
        bagel::MaskBuilder().set<Input>().set<MTransform>().build();

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

    constexpr float Y_MARGIN = 1.5f;

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(locksterMask))
            continue;

        auto&       a  = e.get<LocksterAnimation>();
        const auto& ai = e.get<AI>();
        const auto& t  = e.get<MTransform>();
        auto&       rf = e.get<RenderFrame>();

        const float distX      = std::abs(t.x - playerX);
        const float distY      = std::abs(t.y - playerY);
        const bool  inRange    = distX < ai.detectionRange && distY < Y_MARGIN;
        const bool  isCharging = ai.alertTimer >= 30;

        if (isCharging)
            a.state = LocksterAnimation::State::Charge;
        else if (inRange)
            a.state = LocksterAnimation::State::Alert;
        else
            a.state = LocksterAnimation::State::Idle;

        tickAnim(a, rf);
    }
}

void explosionAnimSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<ExplosionAnimation>().set<RenderFrame>().build();

    std::vector<ent_type> toDestroy;

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        tickAnim(e.get<ExplosionAnimation>(), e.get<RenderFrame>());
        if (e.get<RenderFrame>().finishedThisTick)
            toDestroy.push_back(e.entity());
    }

    for (ent_type e : toDestroy)
        bagel::Entity{e}.destroy();
}

void drawSystem(SDL_Renderer* ren)
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<MTransform>()
                                        .set<Drawable>()
                                        .set<RenderFrame>()
                                        .set<Movement>()
                                        .build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        const auto& t  = e.get<MTransform>();
        const auto& rf = e.get<RenderFrame>();
        const auto& d  = e.get<Drawable>();
        const auto& m  = e.get<Movement>();

        if (d.texture == nullptr)
            continue;

        if (e.has<Health>())
        {
            const auto& h = e.get<Health>();
            if (h.isInvulnerable && (h.invulnerableTimer / 4) % 2 == 0)
                continue;
        }

        SDL_FRect src  = {rf.spriteIndex * d.spriteW, 0.f, d.spriteW, d.spriteH};
        SDL_FRect dest = transformToFrect(t);

        const bool         shouldFlip = m.facingLeft == d.defaultFacingLeft;
        const SDL_FlipMode flip = shouldFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderTextureRotated(ren, d.texture, &src, &dest, 0.0, nullptr, flip);
    }
}

static void destroyProjectile(bagel::Entity ent)
{
    if (ent.has<Movement>())
    {
        b2BodyId bodyId = ent.get<Movement>().bodyId;
        if (b2Body_IsValid(bodyId))
            b2DestroyBody(bodyId);
    }
    ent.destroy();
}

void collisionSystem(b2WorldId world)
{
    constexpr float dt       = 1.f / static_cast<float>(GlobalData::FPS);
    constexpr int   subSteps = 4;
    b2World_Step(world, dt, subSteps);

    static const bagel::Mask playerMask = bagel::MaskBuilder()
                                              .set<Input>()
                                              .set<MTransform>()
                                              .set<Health>()
                                              .set<DamageIntent>()
                                              .build();

    for (bagel::Entity p = bagel::Entity::first(); !p.eof(); p.next())
    {
        if (p.test(playerMask))
        {
            GlobalData::updateCamPosition(p.get<MTransform>().x,
                                          p.get<MTransform>().y);
            break;
        }
    }

    static const bagel::Mask bulletMask =
        bagel::MaskBuilder().set<Projectile>().set<Movement>().build();
    static const bagel::Mask inputMask =
        bagel::MaskBuilder().set<Input>().set<DamageIntent>().build();
    static const bagel::Mask enemyTagMask =
        bagel::MaskBuilder().set<Enemy>().build();
    static const bagel::Mask enemyDamageMask =
        bagel::MaskBuilder().set<Enemy>().set<DamageIntent>().build();

    std::vector<ent_type> toDestroy;

    auto lookupId = [](b2BodyId body) -> int {
        return static_cast<int>(
            reinterpret_cast<uintptr_t>(b2Body_GetUserData(body)));
    };

    auto alreadyQueued = [&toDestroy](ent_type e) -> bool {
        for (const ent_type& x : toDestroy)
            if (x.id == e.id)
                return true;
        return false;
    };

    b2ContactEvents contacts = b2World_GetContactEvents(world);
    for (int i = 0; i < contacts.beginCount; ++i)
    {
        const b2ContactBeginTouchEvent& ev  = contacts.beginEvents[i];
        const int                       idA = lookupId(b2Shape_GetBody(ev.shapeIdA));
        const int                       idB = lookupId(b2Shape_GetBody(ev.shapeIdB));

        if (idA < 0 && idB < 0)
            continue;

        // Identify which side (if any) is a bullet
        int bulletId = -1;
        int otherId  = -1;
        if (idA >= 0)
        {
            bagel::Entity tmp{ent_type{idA}};
            if (tmp.test(bulletMask))
            {
                bulletId = idA;
                otherId  = idB;
            }
        }
        if (bulletId < 0 && idB >= 0)
        {
            bagel::Entity tmp{ent_type{idB}};
            if (tmp.test(bulletMask))
            {
                bulletId = idB;
                otherId  = idA;
            }
        }

        if (bulletId >= 0)
        {
            bagel::Entity bullet{ent_type{bulletId}};

            if (!alreadyQueued(bullet.entity()))
                toDestroy.push_back(bullet.entity());

            if (otherId < 0)
                continue; // hit world geometry — destroy bullet, no damage

            bagel::Entity other{ent_type{otherId}};
            const bool    fromEnemy = bullet.get<Projectile>().fromEnemy;

            if (!fromEnemy && other.test(enemyDamageMask))
                other.get<DamageIntent>() = {.amount      = BULLET_DAMAGE,
                                             .pending     = true,
                                             .fromContact = false};
            else if (fromEnemy && other.test(inputMask))
                other.get<DamageIntent>() = {.amount      = BULLET_DAMAGE,
                                             .pending     = true,
                                             .fromContact = false};
            continue;
        }

        // No bullet — check for player-enemy body contact
        if (idA < 0 || idB < 0)
            continue;

        bagel::Entity entA{ent_type{idA}};
        bagel::Entity entB{ent_type{idB}};

        bagel::Entity* pPlayer = nullptr;
        if (entA.test(inputMask) && entB.test(enemyTagMask))
            pPlayer = &entA;
        else if (entB.test(inputMask) && entA.test(enemyTagMask))
            pPlayer = &entB;

        if (pPlayer)
            pPlayer->get<DamageIntent>() = {.amount      = ENEMY_CONTACT_DAMAGE,
                                            .pending     = true,
                                            .fromContact = true};
    }

    for (ent_type e : toDestroy)
        destroyProjectile(bagel::Entity{e});
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
            h.isInvulnerable    = true;
            h.invulnerableTimer = 90;
            h.justHit           = true;
            if (di.fromContact)
                std::cout << "hit by enemy, hp=" << h.points << "\n";
            else if (di.fromFall)
                std::cout << "player fell, hp=" << h.points << "\n";
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
            h.isInvulnerable        = false;
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

    intent       = {};
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
                  float             playerX,
                  float             playerY)
{
    constexpr float Y_MARGIN = 1.5f;
    const float     distX    = std::abs(t.x - playerX);
    const float     distY    = std::abs(t.y - playerY);

    intent = {};

    const bool inRange    = distX < ai.detectionRange && distY < Y_MARGIN;
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
        }
    }
    else if (inRange)
    {
        ++ai.alertTimer;

        if (ai.alertTimer == 30)
            ai.targetX = playerX;

        if constexpr (LOCKSTER_HAS_BULLETS)
        {
            if (ai.shotsFired < 3 && ai.alertTimer % 10 == 0)
            {
                const float velX = t.x < playerX ? 0.15f : -0.15f;
                createProjectile(GlobalData::getBoxWorld(),
                                 t.x,
                                 t.y,
                                 velX,
                                 0.f,
                                 true);
                ++ai.shotsFired;
            }
        }
    }
    else
    {
        ai.alertTimer = 0;
        ai.shotsFired = 0;
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
            auto& ai     = e.get<AI>();
            auto& intent = e.get<Intent>();

            if (ai.freezeFrames > 0)
            {
                --ai.freezeFrames;
                const float spd = intent.speed;
                intent          = {};
                intent.speed    = spd;
                continue;
            }

            const auto& t = e.get<MTransform>();
            auto&       m = e.get<Movement>();

            switch (ai.type)
            {
            case AI::Type::Patroller:
                tickPatroller(ai, t, intent, playerX, playerY);
                break;
            case AI::Type::Lockster:
                tickLockster(ai, t, m, intent, playerX, playerY);
                break;
            }
        }
    }
}

void checkpointSystem(const std::vector<SpawnPoint>& checkpoints)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Input>().set<MTransform>().set<Respawn>().build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        const auto& t = e.get<MTransform>();
        auto&       r = e.get<Respawn>();

        for (const SpawnPoint& cp : checkpoints)
        {
            if (t.x >= cp.x && cp.x > r.lastCheckpointX)
            {
                r.lastCheckpointX = cp.x;
                r.lastCheckpointY = cp.y;
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
                const float spd    = intent.speed;
                intent             = {};
                intent.speed       = spd;
            }
        };

        if (!r.isRespawning && h.justHit)
        {
            h.justHit      = false;
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
                    destroyProjectile(b);
            }
        }
        else if (r.isRespawning)
        {
            if (--r.flickerTimer <= 0)
            {
                if (b2Body_IsValid(m.bodyId))
                {
                    b2Body_SetTransform(m.bodyId,
                                        {r.lastCheckpointX, r.lastCheckpointY},
                                        b2Body_GetRotation(m.bodyId));
                    transformUpdateWithB2Pos(t, b2Body_GetPosition(m.bodyId));
                    b2Body_SetLinearVelocity(m.bodyId, {0.f, 0.f});
                }
                h.isInvulnerable    = true;
                h.invulnerableTimer = 60;
                r.isRespawning      = false;

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
                    auto& em  = en.get<Movement>();
                    auto& et  = en.get<MTransform>();
                    if (b2Body_IsValid(em.bodyId))
                    {
                        b2Body_SetTransform(em.bodyId,
                                            {eai.spawnX, eai.spawnY},
                                            b2Body_GetRotation(em.bodyId));
                        transformUpdateWithB2Pos(et, b2Body_GetPosition(em.bodyId));
                        b2Body_SetLinearVelocity(em.bodyId, {0.f, 0.f});
                    }
                    eai.alertTimer      = 0;
                    eai.shotsFired      = 0;
                    eai.freezeFrames    = 0;
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

} // namespace megaman
