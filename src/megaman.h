#pragma once

#include "MTransform.h"
#include "SDL3/SDL.h"
#include "bagel.h"
#include <array>
#include <box2d/box2d.h>
#include <cstdint>
#include <string>

namespace megaman
{ // forward declarations
struct AnimationClip;
struct RenderFrame;
struct PlayerAnimation;
struct PatrollerAnimation;
struct LocksterAnimation;
struct ExplosionAnimation;
struct Movement;
struct Jump;
struct Collision;
struct Drawable;
struct Health;
struct Input;
struct Intent;
struct DamageIntent;
struct Enemy;
struct AI;
struct Weapon;
struct Projectile;
struct Respawn;
} // namespace megaman

// ============= STORAGE SPECIALIZATIONS =============

template <>
struct bagel::Storage<megaman::PlayerAnimation> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::PlayerAnimation>;
};

template <>
struct bagel::Storage<megaman::PatrollerAnimation> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::PatrollerAnimation>;
};

template <>
struct bagel::Storage<megaman::LocksterAnimation> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::LocksterAnimation>;
};

template <>
struct bagel::Storage<megaman::ExplosionAnimation> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::ExplosionAnimation>;
};

template <>
struct bagel::Storage<megaman::RenderFrame> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::RenderFrame>;
};

template <>
struct bagel::Storage<megaman::MTransform> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<megaman::MTransform>;
};

template <>
struct bagel::Storage<megaman::Movement> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::Movement>;
};

template <>
struct bagel::Storage<megaman::Jump> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<megaman::Jump>;
};

template <>
struct bagel::Storage<megaman::Collision> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::Collision>;
};

template <>
struct bagel::Storage<megaman::Drawable> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<megaman::Drawable>;
};

template <>
struct bagel::Storage<megaman::Health> final : bagel::NoInstance
{
    using type = bagel::StackStorage<megaman::Health>;
};

template <>
struct bagel::Storage<megaman::Input> final : bagel::NoInstance
{
    using type = bagel::TaggedStorage<megaman::Input>;
};

template <>
struct bagel::Storage<megaman::Intent> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::Intent>;
};

template <>
struct bagel::Storage<megaman::DamageIntent> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::DamageIntent>;
};

template <>
struct bagel::Storage<megaman::Enemy> final : bagel::NoInstance
{
    using type = bagel::TaggedStorage<megaman::Enemy>;
};

template <>
struct bagel::Storage<megaman::AI> final : bagel::NoInstance
{
    using type = bagel::StackStorage<megaman::AI>;
};

template <>
struct bagel::Storage<megaman::Weapon> final : bagel::NoInstance
{
    using type = bagel::StackStorage<megaman::Weapon>;
};

template <>
struct bagel::Storage<megaman::Projectile> final : bagel::NoInstance
{
    using type = bagel::StackStorage<megaman::Projectile>;
};

template <>
struct bagel::Storage<megaman::Respawn> final : bagel::NoInstance
{
    using type = bagel::StackStorage<megaman::Respawn>;
};

namespace megaman
{
using ent_type = bagel::ent_type;

// Collision filter categories — used by all entity create functions and map bodies
inline constexpr uint64_t CAT_WORLD         = 0x0001;
inline constexpr uint64_t CAT_PLAYER        = 0x0002;
inline constexpr uint64_t CAT_ENEMY         = 0x0004;
inline constexpr uint64_t CAT_PLAYER_BULLET = 0x0008;
inline constexpr uint64_t CAT_ENEMY_BULLET  = 0x0010;

// ============= COMPONENTS =============

inline constexpr int ANIM_SPEED = 8;

// One entry in a per-entity clip table. Describes a slice of the sprite sheet
// for a single animation state. Stored in the per-entity animation component
// (PlayerAnimation, LocksterAnimation, etc.) indexed by that entity's State enum.
// frameCount=1 → frozen single frame (no advance). loop=false → one-shot (e.g. explosion).
struct AnimationClip
{
    int  startFrame{};              // first column in the sprite sheet
    int  frameCount{1};             // number of columns this clip spans
    int  framesPerStep{ANIM_SPEED}; // game ticks between frame advances
    bool loop{true}; // wraps to startFrame when done; false = clamp + signal finish
};

// Output of the animation systems, consumed by drawSystem.
// Each animation system writes the resolved sprite-sheet index here so drawSystem
// needs no knowledge of states, clips, or timers — it only reads spriteIndex.
// finishedThisTick is raised for one tick when a non-looping clip plays its last frame
// (used by explosionAnimSystem to know when to destroy the entity).
struct RenderFrame
{
    int spriteIndex{}; // absolute column in the sprite sheet to render this tick
    bool
        finishedThisTick{}; // true for exactly one tick when a non-looping clip ends
};

struct PlayerAnimation
{
    enum class State
    {
        Idle,
        Run
    };
    State                        state{State::Idle};
    State                        prev{State::Idle};
    int                          frame{};
    int                          timer{};
    std::array<AnimationClip, 2> clips{};
};

struct PatrollerAnimation
{
    enum class State
    {
        Run
    };
    State                        state{State::Run};
    State                        prev{State::Run};
    int                          frame{};
    int                          timer{};
    std::array<AnimationClip, 1> clips{};
};

struct LocksterAnimation
{
    enum class State
    {
        Idle,
        Charge,
        Alert
    };
    State                        state{State::Idle};
    State                        prev{State::Idle};
    int                          frame{};
    int                          timer{};
    std::array<AnimationClip, 3> clips{};
};

struct ExplosionAnimation
{
    enum class State
    {
        Playing
    };
    State                        state{State::Playing};
    State                        prev{State::Playing};
    int                          frame{};
    int                          timer{};
    std::array<AnimationClip, 1> clips{};
};

struct Movement
{
    // Preferred storage: Packed, per frame iteration even though entities
    // with movement will be created and destroyed "frequently" so stack is an option too.
    float    speed{};
    float    mass{};
    float    velX{};
    float    velY{};
    float    accX{};
    float    accY{};
    b2BodyId bodyId{};
    bool     facingLeft{};
};

struct Jump
{
    // Preferred storage: Sparse, only player has it

    bool  isGrounded{};
    bool  isJumping{};
    float impulse{};
    float bufferTimer{};
    float coyoteTimer{};
};

struct Collision
{
    // Preferred storage: Packed, same reason as Movement since its also physics related.

    float width{};
    float height{};
};

struct Drawable
{
    // Preferred storage: Sparse, almost every entity can be drawn to the screen.

    SDL_Texture* texture{nullptr};
    float        spriteW{};
    float        spriteH{};
    bool         defaultFacingLeft{true};
};

struct Health
{
    // Preferred storage: Stack, relatively few entities will have health and
    // they are created and destroyed a lot.

    float points{};
    bool  isInvulnerable{};
    bool  isContactInvulnerable{};
    bool  isDead{};
    bool  justHit{};
    int   invulnerableTimer{};
};

struct Input
{
    // Preferred storage: Tagged, we only need to know if entity has it
    // so it can react to input events.
};

struct Intent
{
    // What the entity wants to do this frame.
    // InputSystem writes this for the player; AISystem writes it for enemies.
    // MovementSystem and ShootingSystem consume it.
    bool moveLeft{};
    bool moveRight{};
    bool moveUp{};
    bool moveDown{};
    bool shoot{};
};

struct DamageIntent
{
    // Queued damage written by CollisionSystem, consumed by DamageSystem.
    float amount{};
    bool  pending{};
    bool  fromContact{};
};

struct Enemy
{
    // Preferred storage: Tagged, only need to know if entity has it.
};

struct AI
{
    enum class Type
    {
        Patroller,
        Lockster
    };
    enum State
    {
        PATROL,
        CHASE_SHOOT
    };

    Type  type{};
    State state{};
    float patrolMinX{};
    float patrolMaxX{};
    float detectionRange{};
    int   alertTimer{};
    float chargeSpeed{};
    float spawnX{};
    float spawnY{};
    int   shootCooldown{};
    int   shotsFired{};
    bool  patrollingRight{true};
    float targetX{};
    int   freezeFrames{};
};

struct Weapon
{
    // Preferred storage: Stack, few entities will be able to shoot
    // and be in the scene at the same time.

    int projectileType{-1};
    int shootCooldown{};
};

struct Projectile
{
    bool fromEnemy{};
};

struct Respawn
{
    float spawnX{};
    float spawnY{};
    float maxHp{};
    int   flickerTimer{};
    bool  isRespawning{};
};

// ============= SYSTEMS    =============

void inputSystem();
void movementSystem();
void jumpSystem();
void playerAnimSystem();
void patrollerAnimSystem();
void locksterAnimSystem();
void explosionAnimSystem();
void drawSystem(SDL_Renderer* ren);
void shootingSystem();
void collisionSystem(b2WorldId box);
void damageSystem();
void healthSystem();
void aiSystem();
void respawnSystem();

// ============= ENTITIES   =============

ent_type createPlayer(b2WorldId world, float x, float y, int hp, SDL_Texture* tex);

ent_type createPatroller(b2WorldId    world,
                         float        x,
                         float        y,
                         float        hp,
                         float        patrolMinX,
                         float        patrolMaxX,
                         float        detectionRange,
                         float        speed,
                         SDL_Texture* tex);

ent_type createLockster(b2WorldId    world,
                        float        x,
                        float        y,
                        float        hp,
                        float        detectionRange,
                        float        chargeSpeed,
                        SDL_Texture* tex);

ent_type createBoss(float x, float y, float hp);

ent_type createPlatform(float x, float y, bool isMoving);

ent_type createProjectile(b2WorldId world,
                          float     x,
                          float     y,
                          float     velX,
                          float     velY,
                          bool      fromEnemy);

ent_type createExplosion(float x, float y);

ent_type createTrigger(float x, float y, float width, float height);

ent_type createItem(float x, float y);

} // namespace megaman
