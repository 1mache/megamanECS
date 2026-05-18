#include "Megaman.h"
#include "GlobalData.h"
#include "MTransform.h"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

namespace
{
// --- Player ---
constexpr float PLAYER_SPRITE_W          = 28.f;
constexpr float PLAYER_SPRITE_H          = 28.f;
constexpr int   PLAYER_IDLE_START        = 8;
constexpr int   PLAYER_IDLE_COUNT        = 1;
constexpr int   PLAYER_RUN_START         = 0;
constexpr int   PLAYER_RUN_COUNT         = 4;
constexpr int   PLAYER_JUMP_START        = 10;
constexpr int   PLAYER_JUMP_COUNT        = 1;
constexpr int   PLAYER_SHOOT_IDLE_START  = 9;
constexpr int   PLAYER_SHOOT_IDLE_COUNT  = 1;
constexpr int   PLAYER_SHOOT_RUN_START   = 4;
constexpr int   PLAYER_SHOOT_RUN_COUNT   = 4;
constexpr int   PLAYER_SHOOT_HOLD_TICKS  = 12;
constexpr float PLAYER_RUN_SPEED         = 10.f;
constexpr float PLAYER_JUMP_IMPULSE      = 40.f;
constexpr float PLAYER_JUMP_COYOTE_TIME  = 40.f;
constexpr float PLAYER_JUMP_BUFFER_TIME  = 10.f;
constexpr float PLAYER_JUMP_FALL_FACTOR  = 3.f;
constexpr float PLAYER_HP                = 3.f;
constexpr int   PLAYER_HIT_IFRAMES       = 90;
constexpr int   PLAYER_RESPAWN_IFRAMES   = 60;
constexpr int   PLAYER_HIT_FREEZE_FRAMES = 60;

// --- Patroller enemy ---
constexpr float PATROLLER_SPRITE_W        = 24.f;
constexpr float PATROLLER_SPRITE_H        = 24.f;
constexpr int   PATROLLER_IDLE_START      = 0;
constexpr int   PATROLLER_IDLE_COUNT      = 1;
constexpr int   PATROLLER_RUN_START       = 0;
constexpr int   PATROLLER_RUN_COUNT       = 2;
constexpr int   PATROLLER_JUMP_START      = 0;
constexpr int   PATROLLER_JUMP_COUNT      = 1;
constexpr int   PATROLLER_ANIM_SPEED      = 16;
constexpr float PATROLLER_DETECTION_RANGE = 15.f;
constexpr float PATROLLER_SPEED           = 5.f;
constexpr int   PATROLLER_HP              = 1.f;
constexpr int   PATROLLER_BURST_START     = 3;
constexpr int   PATROLLER_BURST_COUNT     = 1;
constexpr int   PATROLLER_BURST_FRAMES    = 24;
constexpr int   PATROLLER_BURST_COOLDOWN  = 90;
constexpr float PATROLLER_CENTER_TOL      = 0.4f;

// --- Lockster enemy ---
constexpr float LOCKSTER_SPRITE_W        = 24.f;
constexpr float LOCKSTER_SPRITE_H        = 24.f;
constexpr int   LOCKSTER_IDLE_START      = 0;
constexpr int   LOCKSTER_IDLE_COUNT      = 2;
constexpr int   LOCKSTER_ALERT_START     = 2;
constexpr int   LOCKSTER_ALERT_COUNT     = 3;
constexpr int   LOCKSTER_CHARGE_START    = 5;
constexpr int   LOCKSTER_CHARGE_COUNT    = 4;
constexpr int   LOCKSTER_ANIM_SPEED      = 16;
constexpr float LOCKSTER_DETECTION_RANGE = 8.f;
constexpr float LOCKSTER_CHARGE_SPEED    = 8.f;
constexpr float LOCKSTER_HP              = 0.5f;

// --- Boss ---
constexpr float BOSS_SPRITE_W          = 32.f;
constexpr float BOSS_SPRITE_H          = 32.f;
constexpr int   BOSS_IDLE_ANIM_START   = 0;
constexpr int   BOSS_IDLE_ANIM_COUNT   = 1;
constexpr int   BOSS_CHARGE_ANIM_START = 1;
constexpr int   BOSS_CHARGE_ANIM_COUNT = 1;
constexpr int   BOSS_DASH_ANIM_START   = 2;
constexpr int   BOSS_DASH_ANIM_COUNT   = 3;
constexpr int   BOSS_BRAKE_ANIM_START  = 5;
constexpr int   BOSS_BRAKE_ANIM_COUNT  = 1;
constexpr int   BOSS_SHOOT_ANIM_START  = 6;
constexpr int   BOSS_SHOOT_ANIM_COUNT  = 2;
constexpr int   BOSS_DIE_ANIM_START    = 8;
constexpr int   BOSS_DIE_ANIM_COUNT    = 4;
constexpr int   BOSS_ANIM_SPEED        = 10;
constexpr float BOSS_HP                = 10.f;
constexpr int   BOSS_IDLE_TICKS        = 60;
constexpr int   BOSS_CHARGE_TICKS      = 50;
constexpr int   BOSS_DASH_TICKS        = 45;
constexpr float BOSS_DASH_SPEED        = 20.f;
constexpr int   BOSS_SHOTS             = 2;

// --- Projectile ---
constexpr float PLAYER_SHOT_SPEED    = 0.5f;
constexpr float PATROLLER_SHOT_SPEED = 0.3f;
constexpr float BOSS_SHOT_SPEED      = 0.25f;
constexpr float SHOT_SPRITE_W        = 8.f;
constexpr float SHOT_SPRITE_H        = 8.f;
constexpr float BULLET_HALF_W        = 0.2f;
constexpr float BULLET_HALF_H        = 0.1f;
constexpr float BOSS_SHOT_SPRITE_H   = 10.f; // boss shot is taller than normal

// --- Explosion ---
constexpr float EXPLOSION_SPRITE_W = 22.f;
constexpr float EXPLOSION_SPRITE_H = 24.f;
constexpr int   EXPLOSION_FRAMES   = 3;

// --- Damage ---
constexpr float BULLET_DAMAGE        = 0.5f;
constexpr float BOSS_BULLET_DAMAGE   = 1.f;
constexpr float ENEMY_CONTACT_DAMAGE = 1.f;

// --- Shoot cooldowns ---
constexpr int PLAYER_SHOOT_COOLDOWN    = 20;
constexpr int PATROLLER_SHOOT_COOLDOWN = 120;
constexpr int BOSS_SHOOT_COOLDOWN      = 40;
} // namespace

namespace megaman
{
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

const bagel::Mask& playerMask()
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<Input>()
                                        .set<Jump>()
                                        .set<Movement>()
                                        .set<Weapon>()
                                        .set<PlayerAnimation>()
                                        .set<RenderFrame>()
                                        .set<Drawable>()
                                        .set<MTransform>()
                                        .set<Health>()
                                        .set<DamageIntent>()
                                        .build();
    return mask;
}

ent_type createPlayer(b2WorldId world, float x, float y, SDL_Texture* tex)
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
        {.clips = {AnimationClip{PLAYER_IDLE_START, PLAYER_IDLE_COUNT},
                   AnimationClip{PLAYER_RUN_START, PLAYER_RUN_COUNT},
                   AnimationClip{PLAYER_JUMP_START, PLAYER_JUMP_COUNT},
                   AnimationClip{PLAYER_SHOOT_IDLE_START, PLAYER_SHOOT_IDLE_COUNT},
                   AnimationClip{PLAYER_SHOOT_RUN_START, PLAYER_SHOOT_RUN_COUNT}}});
    bagel::World::addComponent<RenderFrame>(ent, {});
    bagel::World::addComponent<Drawable>(
        ent,
        {.texture = tex, .spriteW = PLAYER_SPRITE_W, .spriteH = PLAYER_SPRITE_H});
    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(
        ent,
        {.speed = PLAYER_RUN_SPEED, .mass = 1.f, .bodyId = body});
    bagel::World::addComponent<Jump>(ent,
                                     {
                                         .impulse     = PLAYER_JUMP_IMPULSE,
                                         .bufferTimer = PLAYER_JUMP_BUFFER_TIME,
                                         .coyoteTimer = PLAYER_JUMP_COYOTE_TIME,
                                     });
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = PLAYER_HP});
    bagel::World::addComponent<DamageIntent>(ent, {});
    bagel::World::addComponent<Input>(ent, {});
    bagel::World::addComponent<Intent>(ent, {});
    bagel::World::addComponent<Weapon>(ent,
                                       {.type          = Weapon::Normal,
                                        .damage        = BULLET_DAMAGE,
                                        .shotSpeed     = PLAYER_SHOT_SPEED,
                                        .shootCooldown = 0});
    bagel::World::addComponent<Respawn>(ent,
                                        {.spawnX          = x,
                                         .spawnY          = y,
                                         .lastCheckpointX = x,
                                         .lastCheckpointY = y,
                                         .maxHp           = PLAYER_HP});

    return ent;
}

ent_type createPatroller(b2WorldId    world,
                         float        x,
                         float        y,
                         float        patrolMinX,
                         float        patrolMaxX,
                         SDL_Texture* tex)
{
    constexpr float halfW = PATROLLER_SPRITE_W / (2 * GlobalData::PTM);
    constexpr float halfH = PATROLLER_SPRITE_H / (2 * GlobalData::PTM);

    ent_type ent = bagel::World::createEntity();

    b2BodyDef bodyDef     = b2DefaultBodyDef();
    bodyDef.type          = b2_kinematicBody;
    bodyDef.position      = {x, y};
    bodyDef.fixedRotation = true;
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
        {.clips = {AnimationClip{PATROLLER_RUN_START,
                                 PATROLLER_RUN_COUNT,
                                 PATROLLER_ANIM_SPEED}, // [0] Run
                   AnimationClip{PATROLLER_BURST_START,
                                 PATROLLER_BURST_COUNT,
                                 PATROLLER_ANIM_SPEED}}}); // [1] Burst
    bagel::World::addComponent<RenderFrame>(ent, {});
    bagel::World::addComponent<Drawable>(ent,
                                         {.texture           = tex,
                                          .spriteW           = PATROLLER_SPRITE_W,
                                          .spriteH           = PATROLLER_SPRITE_H,
                                          .defaultFacingLeft = false});
    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(
        ent,
        {.speed = PATROLLER_SPEED, .mass = 1, .bodyId = body});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = PATROLLER_HP});
    bagel::World::addComponent<DamageIntent>(ent, {});
    bagel::World::addComponent<Intent>(ent, {});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<AI>(ent,
                                   {.type           = AI::Type::Patroller,
                                    .patrolMinX     = patrolMinX,
                                    .patrolMaxX     = patrolMaxX,
                                    .detectionRange = PATROLLER_DETECTION_RANGE,
                                    .spawnX         = x,
                                    .spawnY         = y});
    bagel::World::addComponent<Weapon>(ent,
                                       {.type          = Weapon::Normal,
                                        .damage        = BULLET_DAMAGE,
                                        .shotSpeed     = PATROLLER_SHOT_SPEED,
                                        .shootCooldown = PATROLLER_SHOOT_COOLDOWN});

    return ent;
}

ent_type createLockster(b2WorldId world, float x, float y, SDL_Texture* tex)
{
    constexpr float halfW = LOCKSTER_SPRITE_W / (2 * GlobalData::PTM);
    constexpr float halfH = LOCKSTER_SPRITE_H / (2 * GlobalData::PTM);

    ent_type ent = bagel::World::createEntity();

    b2BodyDef bodyDef     = b2DefaultBodyDef();
    bodyDef.type          = b2_dynamicBody;
    bodyDef.position      = {x, y};
    bodyDef.fixedRotation = true;
    bodyDef.gravityScale  = 1.f;
    bodyDef.enableSleep   = false;
    bodyDef.userData      = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.id));
    b2BodyId body         = b2CreateBody(world, &bodyDef);

    b2ShapeDef shapeDef                 = b2DefaultShapeDef();
    shapeDef.density                    = 1.f;
    shapeDef.material.friction          = 0.f;
    shapeDef.material.restitution       = 1.f;
    shapeDef.material.rollingResistance = 0.f;
    shapeDef.material.tangentSpeed      = 0.f;
    shapeDef.enableContactEvents        = true;
    shapeDef.filter.categoryBits        = CAT_ENEMY;
    shapeDef.filter.maskBits            = CAT_WORLD | CAT_PLAYER | CAT_PLAYER_BULLET;
    b2Polygon boxShape                  = b2MakeBox(halfW, halfH);
    b2CreatePolygonShape(body, &shapeDef, &boxShape);

    bagel::World::addComponent<LocksterAnimation>(
        ent,
        {.clips = {AnimationClip{LOCKSTER_IDLE_START,
                                 LOCKSTER_IDLE_COUNT,
                                 LOCKSTER_ANIM_SPEED}, // [0] Idle
                   AnimationClip{LOCKSTER_CHARGE_START,
                                 LOCKSTER_CHARGE_COUNT}, // [1] Charge
                   AnimationClip{LOCKSTER_ALERT_START,
                                 LOCKSTER_ALERT_COUNT,
                                 LOCKSTER_ANIM_SPEED}}}); // [2] Alert
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
    bagel::World::addComponent<Health>(ent, {.points = LOCKSTER_HP});
    bagel::World::addComponent<DamageIntent>(ent, {});
    bagel::World::addComponent<Intent>(ent, {});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<AI>(ent,
                                   {.type           = AI::Type::Lockster,
                                    .detectionRange = LOCKSTER_DETECTION_RANGE,
                                    .spawnX         = x,
                                    .spawnY         = y});
    return ent;
}

ent_type createBoss(b2WorldId world, float x, float y, SDL_Texture* tex)
{
    constexpr float halfW = BOSS_SPRITE_W / (2 * GlobalData::PTM);
    constexpr float halfH = BOSS_SPRITE_H / (2 * GlobalData::PTM);

    ent_type ent = bagel::World::createEntity();

    b2BodyDef bodyDef     = b2DefaultBodyDef();
    bodyDef.type          = b2_dynamicBody;
    bodyDef.position      = {x, y};
    bodyDef.fixedRotation = true;
    bodyDef.gravityScale  = 1.f;
    bodyDef.enableSleep   = false;
    bodyDef.userData      = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.id));
    b2BodyId body         = b2CreateBody(world, &bodyDef);

    b2ShapeDef shapeDef          = b2DefaultShapeDef();
    shapeDef.density             = 1.f;
    shapeDef.enableContactEvents = true;
    shapeDef.filter.categoryBits = CAT_ENEMY;
    shapeDef.filter.maskBits     = CAT_WORLD | CAT_PLAYER | CAT_PLAYER_BULLET;
    constexpr float HITBOX_SHRINK_FACTOR = 0.85f;
    // smaller hitbox than sprite for easier dodging
    b2Polygon boxShape =
        b2MakeOffsetBox(halfW,
                        halfH * HITBOX_SHRINK_FACTOR,
                        {0.f, -halfH * (1.f - HITBOX_SHRINK_FACTOR)},
                        b2Rot_identity);
    b2CreatePolygonShape(body, &shapeDef, &boxShape);

    bagel::World::addComponent<BossAnimation>(
        ent,
        {.clips = {AnimationClip{BOSS_IDLE_ANIM_START,
                                 BOSS_IDLE_ANIM_COUNT,
                                 BOSS_ANIM_SPEED}, // [0] IDLE
                   AnimationClip{BOSS_CHARGE_ANIM_START,
                                 BOSS_CHARGE_ANIM_COUNT,
                                 BOSS_ANIM_SPEED}, // [1] CHARGE_DASH
                   AnimationClip{BOSS_DASH_ANIM_START,
                                 BOSS_DASH_ANIM_COUNT,
                                 BOSS_ANIM_SPEED,
                                 false}, // [2] DASH
                   AnimationClip{BOSS_SHOOT_ANIM_START,
                                 BOSS_SHOOT_ANIM_COUNT,
                                 BOSS_ANIM_SPEED,
                                 false}, // [3] SHOOT (one-shot)
                   AnimationClip{BOSS_DIE_ANIM_START,
                                 BOSS_DIE_ANIM_COUNT,
                                 BOSS_ANIM_SPEED,
                                 false}, // [4] DIE (one-shot)
                   AnimationClip{BOSS_BRAKE_ANIM_START,
                                 BOSS_BRAKE_ANIM_COUNT,
                                 BOSS_ANIM_SPEED,
                                 false}}}); // [5] BRAKE, one-shot
    bagel::World::addComponent<RenderFrame>(ent, {});
    bagel::World::addComponent<Drawable>(ent,
                                         {.texture           = tex,
                                          .spriteW           = BOSS_SPRITE_W,
                                          .spriteH           = BOSS_SPRITE_H,
                                          .defaultFacingLeft = false});
    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(ent, {.mass = 1, .bodyId = body});
    bagel::World::addComponent<Collision>(ent, {});
    bagel::World::addComponent<Health>(ent, {.points = BOSS_HP});
    bagel::World::addComponent<DamageIntent>(ent, {});
    bagel::World::addComponent<Intent>(ent, {});
    bagel::World::addComponent<Enemy>(ent, {});
    bagel::World::addComponent<BossAI>(ent, {.stateTimer = BOSS_IDLE_TICKS});
    bagel::World::addComponent<Weapon>(ent,
                                       {.type          = Weapon::Boss,
                                        .damage        = BOSS_BULLET_DAMAGE,
                                        .shotSpeed     = BOSS_SHOT_SPEED,
                                        .shootCooldown = BOSS_SHOOT_COOLDOWN});

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

ent_type createProjectile(b2WorldId    world,
                          float        x,
                          float        y,
                          float        velX,
                          float        velY,
                          Weapon::Type type,
                          float        damage,
                          bool         fromEnemy)
{
    constexpr float fps     = static_cast<float>(GlobalData::FPS);
    const bool      isBoss  = type == Weapon::Boss;
    const float     spriteH = isBoss ? BOSS_SHOT_SPRITE_H : SHOT_SPRITE_H;
    const float     halfW =
        isBoss ? SHOT_SPRITE_W / (2.f * GlobalData::PTM) : BULLET_HALF_W;
    const float halfH =
        isBoss ? BOSS_SHOT_SPRITE_H / (2.f * GlobalData::PTM) : BULLET_HALF_H;

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
    b2Polygon boxShape = b2MakeBox(halfW, halfH);
    b2CreatePolygonShape(body, &shapeDef, &boxShape);

    bagel::World::addComponent<Drawable>(ent,
                                         {.texture = GlobalData::getShotTexture(),
                                          .spriteW = SHOT_SPRITE_W,
                                          .spriteH = spriteH});
    bagel::World::addComponent<RenderFrame>(ent,
                                            {.spriteIndex = static_cast<int>(type)});
    bagel::World::addComponent<MTransform>(ent,
                                           {.x = x, .y = y, .w = halfW, .h = halfH});
    bagel::World::addComponent<Movement>(
        ent,
        {.mass = 1, .velX = velX, .velY = velY, .bodyId = body});
    bagel::World::addComponent<Projectile>(
        ent,
        {.type = type, .damage = damage, .fromEnemy = fromEnemy});

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
                                 DEFAULT_ANIM_SPEED,
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

            m.velX = intent.moveLeft ? -m.speed : intent.moveRight ? m.speed : 0.f;

            auto currentVel = b2Body_GetLinearVelocity(m.bodyId);


            if (b2Body_IsValid(m.bodyId))
                b2Body_SetLinearVelocity(m.bodyId, {m.velX, currentVel.y});
        }

        if (b2Body_IsValid(m.bodyId))
            transformUpdateWithB2Pos(t, b2Body_GetPosition(m.bodyId));

        if (e.has<Health>() && t.y < sceneMinY)
        {
            auto& h = e.get<Health>();
            if (e.test(playerMask()))
            {
                e.get<DamageIntent>() = {.amount      = h.points,
                                         .pending     = true,
                                         .fromContact = false,
                                         .fromFall    = true};
            }
            else
            {
                if (b2Body_IsValid(m.bodyId))
                    b2DestroyBody(m.bodyId);
                m.bodyId = b2_nullBodyId;
                bagel::Entity{e.entity()}.destroy();
            }
        }
    }
}

void jumpSystem()
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<Jump>()
                                        .set<MTransform>()
                                        .set<Movement>()
                                        .set<Intent>()
                                        .build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        auto& j      = e.get<Jump>();
        auto& m      = e.get<Movement>();
        auto& t      = e.get<MTransform>();
        auto& intent = e.get<Intent>();

        auto& bodyId = m.bodyId;

        // how far rays go past sprite bottom.
        constexpr float     GROUND_PROBE = 0.05f;
        constexpr float     EPSILON      = 0.1f;
        constexpr float     DT           = GlobalData::FRAME_DELTA_MS;
        const b2WorldId     worldId      = GlobalData::getBoxWorld();
        const b2QueryFilter filter       = {.categoryBits = CAT_PLAYER,
                                            .maskBits     = CAT_WORLD};
        const b2Vec2        down         = {0.f, -(t.h + GROUND_PROBE)};
        const float         xs[3] = {t.x, t.x + t.w - EPSILON, t.x - t.w + EPSILON};

        bool wasGrounded = j.isGrounded;

        j.isGrounded = false;
        // check if grounded with 3 raycasts down.
        for (float x : xs)
        {
            b2RayResult res =
                b2World_CastRayClosest(worldId, {x, t.y}, down, filter);
            if (res.hit)
            {
                j.isGrounded = true;
                break;
            }
        }

        // if were on ground reset coyote timer
        if (j.isGrounded)
        {
            // reset gravity in case we were falling
            b2Body_SetGravityScale(bodyId, 1.f);
            j.coyoteTimer = 0.f;
            if (!wasGrounded) // just landed
                j.isJumping = false;
        }
        // if we just left the ground but not by jumping start coyote timer
        else if (wasGrounded && !j.isJumping)
        {
            // if we just left the ground start coyote timer
            j.coyoteTimer = PLAYER_JUMP_COYOTE_TIME;
        }

        const bool jumpPressed = intent.moveUp && !j.prevMoveUp;

        // set buffer only on fresh press while airborne, not while held
        if (jumpPressed && j.isJumping)
            j.bufferTimer = PLAYER_JUMP_BUFFER_TIME;

        if (!j.isJumping)
        {
            if (jumpPressed && (j.isGrounded || j.coyoteTimer > 0.f))
            {
                b2Body_ApplyLinearImpulse(m.bodyId,
                                          {0.f, j.impulse},
                                          {t.x, t.y},
                                          true);
                j.isJumping = true;
            }
            else if (j.isGrounded && j.bufferTimer > 0.f)
            {
                // buffered jump: pressed early while airborne, fires on landing
                b2Body_ApplyLinearImpulse(m.bodyId,
                                          {0.f, j.impulse},
                                          {t.x, t.y},
                                          true);
                j.isJumping   = true;
                j.bufferTimer = 0.f;
            }
        }

        j.prevMoveUp = intent.moveUp;

        auto playerVel = b2Body_GetLinearVelocity(m.bodyId);
        // if we are  in falling phase of jump. increase gravity pull on player. better feel
        if (j.isJumping && playerVel.y <= 0.f)
        {
            b2Body_SetGravityScale(m.bodyId, PLAYER_JUMP_FALL_FACTOR);
        }

        // decrease both timers
        j.coyoteTimer = std::max(0.f, j.coyoteTimer - DT);
        j.bufferTimer = std::max(0.f, j.bufferTimer - DT);
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
            float bVelX = e.get<Movement>().facingLeft ? -w.shotSpeed : w.shotSpeed;

            createProjectile(GlobalData::getBoxWorld(),
                             t.x,
                             t.y,
                             bVelX,
                             0.f,
                             w.type,
                             w.damage,
                             fromEnemy);
            if (!fromEnemy && e.has<PlayerAnimation>())
                e.get<PlayerAnimation>().shootHoldTicks = PLAYER_SHOOT_HOLD_TICKS;
            w.shootCooldown =
                fromEnemy ? PATROLLER_SHOOT_COOLDOWN : PLAYER_SHOOT_COOLDOWN;
        }
    }
}

void playerAnimSystem()
{
    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(playerMask()))
            continue;

        auto&       a    = e.get<PlayerAnimation>();
        const auto& m    = e.get<Movement>();
        auto&       rf   = e.get<RenderFrame>();
        auto&       jump = e.get<Jump>();

        if (a.shootHoldTicks > 0)
            --a.shootHoldTicks;

        if (jump.isJumping)
            a.state = PlayerAnimation::State::Jump;
        else if (a.shootHoldTicks > 0)
            a.state = m.velX != 0.f ? PlayerAnimation::State::ShootRun
                                    : PlayerAnimation::State::ShootIdle;
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
    float                    playerX      = 0.f;
    float                    playerY      = 0.f;
    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(playerMask()))
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

        SDL_FRect src  = {static_cast<float>(rf.spriteIndex) * d.spriteW,
                          0.f,
                          d.spriteW,
                          d.spriteH};
        SDL_FRect dest = transformToFrect(t);

        const bool         shouldFlip = m.facingLeft == d.defaultFacingLeft;
        const SDL_FlipMode flip = shouldFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderTextureRotated(ren, d.texture, &src, &dest, 0.0, nullptr, flip);
    }
}

void hudSystem(SDL_Renderer* ren, SDL_Texture* heartTex)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Input>().set<Health>().set<Respawn>().build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        const auto& h = e.get<Health>();
        const auto& r = e.get<Respawn>();

        const float hp      = std::max(0.f, h.points);
        const int   nFull   = static_cast<int>(hp);
        const bool  hasHalf = (hp - static_cast<float>(nFull)) >= 0.5f;
        const int   total   = static_cast<int>(r.maxHp);

        constexpr float HEART_SPRITE_SIDE = 8;
        constexpr float HEART_DST         = 24.f;
        constexpr float HEART_GAP         = 4.f;
        constexpr float HEART_X0          = 10.f;
        constexpr float HEART_Y0          = 10.f;

        for (int i = 0; i < total; ++i)
        {
            const float x =
                HEART_X0 + static_cast<float>(i) * (HEART_DST + HEART_GAP);

            SDL_FRect src = {0.f, 0.f, HEART_SPRITE_SIDE, HEART_SPRITE_SIDE};
            SDL_FRect dst = {x, HEART_Y0, HEART_DST, HEART_DST};

            if (i < nFull)
            {
                SDL_RenderTexture(ren, heartTex, &src, &dst);
            }
            else if (i == nFull && hasHalf)
            {
                // draw opaque bg heart
                SDL_SetTextureAlphaMod(heartTex, 80);
                SDL_RenderTexture(ren, heartTex, &src, &dst);
                SDL_SetTextureAlphaMod(heartTex, 255);
                // on top of that half heart
                src = {0.f, 0.f, HEART_SPRITE_SIDE / 2.f, HEART_SPRITE_SIDE};
                dst = {x, HEART_Y0, HEART_DST / 2.f, HEART_DST};
                SDL_RenderTexture(ren, heartTex, &src, &dst);
            }
            else
            {
                SDL_SetTextureAlphaMod(heartTex, 80);
                SDL_RenderTexture(ren, heartTex, &src, &dst);
                SDL_SetTextureAlphaMod(heartTex, 255);
            }
        }

        break;
    }
}

namespace
{
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
} // namespace

void projectileCullSystem()
{
    const WorldBoundsM bounds = GlobalData::getCamBoundsM();

    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Projectile>().set<MTransform>().build();

    std::vector<ent_type> toDestroy;
    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;
        const auto& t = e.get<MTransform>();
        if (t.x < bounds.minX - BULLET_HALF_W || t.x > bounds.maxX + BULLET_HALF_W ||
            t.y < bounds.minY - BULLET_HALF_H || t.y > bounds.maxY + BULLET_HALF_H)
            toDestroy.push_back(e.entity());
    }
    for (ent_type id : toDestroy)
        destroyProjectile(bagel::Entity{id});
}

void collisionSystem(b2WorldId world)
{
    constexpr float dt       = 1.f / static_cast<float>(GlobalData::FPS);
    constexpr int   subSteps = 4;
    b2World_Step(world, dt, subSteps);

    for (bagel::Entity p = bagel::Entity::first(); !p.eof(); p.next())
    {
        if (p.test(playerMask()))
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
            if (otherId < 0)
            {
                if (!alreadyQueued(bullet.entity()))
                    toDestroy.push_back(bullet.entity());
                continue; // hit world geometry. destroy bullet, no damage
            }
            bagel::Entity other{ent_type{otherId}};

            // bounce bullet back if hit invulnerable enemy
            if (other.has<Health>() && other.has<Enemy>() &&
                other.get<Health>().isInvulnerable)
            {
                auto& bulletM = bullet.get<Movement>();
                bulletM.velX  = -bulletM.velX;
                continue;
            }
            else if (!alreadyQueued(bullet.entity()))
                toDestroy.push_back(bullet.entity());


            const bool fromEnemy = bullet.get<Projectile>().fromEnemy;

            const float dmg = bullet.get<Projectile>().damage;
            if (!fromEnemy && other.test(enemyDamageMask))
                other.get<DamageIntent>() = {.amount      = dmg,
                                             .pending     = true,
                                             .fromContact = false};
            else if (fromEnemy && other.test(inputMask))
                other.get<DamageIntent>() = {.amount      = dmg,
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
        if (!h.isInvulnerable)
        {
            h.points -= di.amount;

            if (di.fromContact)
                std::cout << "hit by enemy, hp=" << h.points << "\n";
            else if (di.fromFall)
                std::cout << "player fell, hp=" << h.points << "\n";
            else
                std::cout << (e.has<Input>() ? "enemy bullet hit player"
                                             : "player bullet hit enemy")
                          << ", hp=" << h.points << "\n";

            // if its player
            if (e.has<Input>())
            {
                static const bagel::Mask aiMask =
                    bagel::MaskBuilder().set<AI>().build();
                for (bagel::Entity en = bagel::Entity::first(); !en.eof(); en.next())
                {
                    if (en.test(aiMask))
                        en.get<AI>().freezeFrames = PLAYER_HIT_FREEZE_FRAMES;
                }

                h.isInvulnerable    = true;
                h.invulnerableTimer = PLAYER_HIT_IFRAMES;
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
        }
        if (h.points <= 0)
        {
            if (e.has<BossAI>())
            {
                auto& bossAi = e.get<BossAI>();
                if (bossAi.state != BossAI::State::DIE)
                    bossAi.state = BossAI::State::DIE;
            }
            else if (e.has<Enemy>())
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
void fireRadialBurst(const MTransform& t, bagel::Entity self)
{
    const Weapon&                                    w    = self.get<Weapon>();
    constexpr float                                  k    = 0.70710678f;
    constexpr std::array<std::pair<float, float>, 8> dirs = {{{1.f, 0.f},
                                                              {k, k},
                                                              {0.f, 1.f},
                                                              {-k, k},
                                                              {-1.f, 0.f},
                                                              {-k, -k},
                                                              {0.f, -1.f},
                                                              {k, -k}}};
    for (auto [dx, dy] : dirs)
    {
        createProjectile(GlobalData::getBoxWorld(),
                         t.x,
                         t.y,
                         dx * w.shotSpeed,
                         dy * w.shotSpeed,
                         w.type,
                         w.damage,
                         true);
    }
}

void tickPatroller(bagel::Entity& patroller, float playerX, float playerY)
{
    AI&               ai     = patroller.get<AI>();
    const MTransform& t      = patroller.get<MTransform>();
    Intent&           intent = patroller.get<Intent>();

    intent = {};

    if (ai.burstCooldown > 0)
        --ai.burstCooldown;

    if (ai.burstTimer > 0)
    {
        --ai.burstTimer;
        if (!ai.burstFired)
        {
            fireRadialBurst(t, patroller);
            ai.burstFired = true;
        }
        ai.state = AI::BURST;
        if (patroller.has<PatrollerAnimation>())
            patroller.get<PatrollerAnimation>().state =
                PatrollerAnimation::State::Burst;
        return;
    }

    ai.state = AI::PATROL;
    if (patroller.has<PatrollerAnimation>())
        patroller.get<PatrollerAnimation>().state = PatrollerAnimation::State::Run;

    if (t.x <= ai.patrolMinX)
        ai.patrollingRight = true;
    else if (t.x >= ai.patrolMaxX)
        ai.patrollingRight = false;
    if (ai.patrollingRight)
        intent.moveRight = true;
    else
        intent.moveLeft = true;

    const float distX  = std::abs(t.x - playerX);
    const float distY  = std::abs(t.y - playerY);
    const float center = 0.5f * (ai.patrolMinX + ai.patrolMaxX);
    const bool  inRange =
        (distX * distX + distY * distY) <
        (ai.detectionRange * ai.detectionRange); // circular range check
    const bool atCenter = std::abs(t.x - center) < PATROLLER_CENTER_TOL;
    if (inRange && atCenter && ai.burstCooldown == 0)
    {
        ai.burstTimer    = PATROLLER_BURST_FRAMES;
        ai.burstFired    = false;
        ai.burstCooldown = PATROLLER_BURST_COOLDOWN;
        intent.moveLeft = intent.moveRight = false;
    }
}

void tickLockster(bagel::Entity& lockster, float playerX, float playerY)
{
    AI&               ai     = lockster.get<AI>();
    const MTransform& t      = lockster.get<MTransform>();
    Movement&         m      = lockster.get<Movement>();
    Intent&           intent = lockster.get<Intent>();
    Health&           h      = lockster.get<Health>();

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
            m.speed       = 0.f;
        }
        else
        {
            m.speed = LOCKSTER_CHARGE_SPEED;
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
    }
    else
    {
        ai.alertTimer = 0;
    }

    // lockster is invulnerable while idle
    h.isInvulnerable = !isCharging;
}

void tickBoss(bagel::Entity& boss, float playerX, float /*playerY*/)
{
    BossAI&           ai     = boss.get<BossAI>();
    const MTransform& t      = boss.get<MTransform>();
    Movement&         m      = boss.get<Movement>();
    Intent&           intent = boss.get<Intent>();

    intent       = {};
    m.speed      = 0.f;
    m.facingLeft = (t.x > playerX);

    switch (ai.state)
    {
    case BossAI::State::IDLE:
        if (--ai.stateTimer <= 0)
        {
            ai.shotsFired = 0;
            ai.shotTimer  = BOSS_SHOOT_COOLDOWN;
            if (ai.nextIsDash)
            {
                ai.state      = BossAI::State::CHARGE_DASH;
                ai.stateTimer = BOSS_CHARGE_TICKS;
            }
            else
            {
                ai.state      = BossAI::State::SHOOT;
                ai.stateTimer = 0;
            }
            ai.nextIsDash = !ai.nextIsDash;
        }
        break;

    case BossAI::State::CHARGE_DASH:
        if (ai.stateTimer == BOSS_CHARGE_TICKS)
            ai.dashRight = (t.x < playerX);
        if (--ai.stateTimer <= 0)
        {
            ai.state      = BossAI::State::DASH;
            ai.stateTimer = BOSS_DASH_TICKS;
        }
        break;

    case BossAI::State::DASH:
        m.speed = BOSS_DASH_SPEED;
        if (ai.dashRight)
            intent.moveRight = true;
        else
            intent.moveLeft = true;
        if (--ai.stateTimer <= 0)
        {
            ai.state      = BossAI::State::IDLE;
            ai.stateTimer = BOSS_IDLE_TICKS;
        }
        break;

    case BossAI::State::SHOOT:
        if (--ai.shotTimer <= 0)
        {
            const Weapon& w    = boss.get<Weapon>();
            const float   velX = (t.x < playerX ? 1.f : -1.f) * w.shotSpeed;
            createProjectile(GlobalData::getBoxWorld(),
                             t.x,
                             t.y,
                             velX,
                             0.f,
                             w.type,
                             w.damage,
                             true);
            ++ai.shotsFired;
            ai.shotTimer = BOSS_SHOOT_COOLDOWN;
            if (ai.shotsFired >= BOSS_SHOTS)
            {
                ai.state      = BossAI::State::IDLE;
                ai.stateTimer = BOSS_IDLE_TICKS;
            }
        }
        break;

    case BossAI::State::DIE:
        break;
    }
}
} // namespace

void aiSystem()
{
    static const bagel::Mask enemyMask = bagel::MaskBuilder()
                                             .set<AI>()
                                             .set<Intent>()
                                             .set<MTransform>()
                                             .set<Movement>()
                                             .build();

    float playerX = 0.f;
    float playerY = 0.f;

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(playerMask()))
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

            if (ai.freezeFrames > 0)
            {
                --ai.freezeFrames;
                e.get<Intent>() = {};
                continue;
            }

            const auto& t = e.get<MTransform>();

            switch (ai.type)
            {
            case AI::Type::Patroller:
                tickPatroller(e, playerX, playerY);
                break;
            case AI::Type::Lockster:
                tickLockster(e, playerX, playerY);
                break;
            }
        }
    }
}

void bossSystem()
{
    static const bagel::Mask bossMask = bagel::MaskBuilder()
                                            .set<BossAI>()
                                            .set<Intent>()
                                            .set<MTransform>()
                                            .set<Movement>()
                                            .set<RenderFrame>()
                                            .build();

    float playerX = 0.f;
    float playerY = 0.f;

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(playerMask()))
        {
            playerX = e.get<MTransform>().x;
            playerY = e.get<MTransform>().y;
            break;
        }
    }

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(bossMask))
            tickBoss(e, playerX, playerY);
    }
}

void bossAnimSystem()
{
    static const bagel::Mask bossMask = bagel::MaskBuilder()
                                            .set<BossAnimation>()
                                            .set<BossAI>()
                                            .set<MTransform>()
                                            .set<RenderFrame>()
                                            .build();

    std::vector<ent_type> toDestroy;

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(bossMask))
            continue;

        auto&       anim = e.get<BossAnimation>();
        const auto& ai   = e.get<BossAI>();
        auto&       rf   = e.get<RenderFrame>();

        anim.state = static_cast<BossAnimation::State>(ai.state);
        // switch to BRAKE in last 30% of dash
        if (anim.state == BossAnimation::State::DASH &&
            static_cast<float>(ai.stateTimer) <
                static_cast<float>(BOSS_DASH_TICKS) * 0.30f)
        {
            anim.state = BossAnimation::State::BRAKE;
        }

        tickAnim(anim, rf);

        if (ai.state == BossAI::State::DIE && rf.finishedThisTick)
        {
            const auto& t = e.get<MTransform>();
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
    }

    for (ent_type e : toDestroy)
        bagel::Entity{e}.destroy();
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

void respawnSystem(b2WorldId                      world,
                   const std::vector<SpawnPoint>& enemySpawns,
                   SDL_Texture*                   locksterTex)
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
                e.get<Intent>() = {};
        };

        if (!r.isRespawning && h.isDead)
        {
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
                h.points            = r.maxHp;
                h.isDead            = false;
                h.isInvulnerable    = true;
                h.invulnerableTimer = PLAYER_RESPAWN_IFRAMES;
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
                    eai.burstTimer      = 0;
                    eai.burstCooldown   = 0;
                    eai.burstFired      = false;
                }

                static const bagel::Mask locksterAIMask =
                    bagel::MaskBuilder().set<AI>().set<MTransform>().build();
                for (const SpawnPoint& sp : enemySpawns)
                {
                    if (sp.type != SpawnPoint::Type::Lockster)
                        continue;

                    bool found = false;
                    for (bagel::Entity le = bagel::Entity::first(); !le.eof();
                         le.next())
                    {
                        if (!le.test(locksterAIMask))
                            continue;
                        const AI& lai = le.get<AI>();
                        if (lai.type == AI::Type::Lockster &&
                            std::abs(lai.spawnX - sp.x) < 0.5f &&
                            std::abs(lai.spawnY - sp.y) < 0.5f)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                        createLockster(world, sp.x, sp.y, locksterTex);
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
