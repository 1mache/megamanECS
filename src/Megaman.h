/**
 * @file Megaman.h
 * @brief ECS component definitions, Box2D filter constants, storage specializations,
 *        entity factories, and ECS system declarations for the Megaman game.
 *
 * Architecture overview:
 *  - Components are plain structs; systems are free functions that iterate the ECS world.
 *  - Animation pipeline: *AnimSystem sets AnimT::state, calls tickAnim() which writes
 *    RenderFrame; drawSystem reads only RenderFrame and never touches animation state.
 *  - Physics bridge: every entity with a Box2D body stores its bagel entity id in
 *    body userData (cast to void*). collisionSystem recovers the id via b2Body_GetUserData
 *    and dispatches damage through DamageIntent.
 *  - Damage pipeline: collisionSystem writes DamageIntent → damageSystem applies it →
 *    healthSystem acts on Health::points.
 */
#pragma once

#include "MTransform.h"
#include "SDL3/SDL.h"
#include "SpawnPoint.h"
#include "bagel.h"
#include <array>
#include <box2d/box2d.h>
#include <cstdint>
#include <string>
#include <vector>

namespace megaman
{ // forward declarations
struct AnimationClip;
struct RenderFrame;
struct PlayerAnimation;
struct PatrollerAnimation;
struct LocksterAnimation;
struct ExplosionAnimation;
struct BossAnimation;
struct BossAI;
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
struct bagel::Storage<megaman::BossAnimation> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::BossAnimation>;
};

template <>
struct bagel::Storage<megaman::BossAI> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::BossAI>;
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

/**
 * @name Box2D collision filter categories
 * @{
 * Each entity's shape sets categoryBits to one of these, and maskBits to the OR of
 * categories it should collide with. collisionSystem uses these to identify contact roles
 * (bullet vs. world, player vs. enemy, etc.) without inspecting component masks.
 */
inline constexpr uint64_t CAT_WORLD         = 0x0001; ///< Static map geometry
inline constexpr uint64_t CAT_PLAYER        = 0x0002; ///< Player body
inline constexpr uint64_t CAT_ENEMY         = 0x0004; ///< Enemy bodies
inline constexpr uint64_t CAT_PLAYER_BULLET = 0x0008; ///< Bullets fired by the player
inline constexpr uint64_t CAT_ENEMY_BULLET  = 0x0010; ///< Bullets fired by enemies
/** @} */

// ============= COMPONENTS =============

inline constexpr int DEFAULT_ANIM_SPEED = 8;

/**
 * @brief Describes one animation clip within a sprite-sheet row.
 *
 * Each animation component (PlayerAnimation, BossAnimation, etc.) stores a fixed-size
 * array of clips indexed by the entity's State enum. tickAnim<> selects the active clip
 * by casting the current state to std::size_t.
 *
 * @note frameCount = 1 → frozen frame (timer and frame counter still run but frame never advances).
 * @note loop = false → one-shot: on the last frame finishedThisTick is raised for exactly one tick.
 */
struct AnimationClip
{
    int  startFrame{};                      // first column in the sprite sheet
    int  frameCount{1};                     // number of columns this clip spans
    int  framesPerStep{DEFAULT_ANIM_SPEED}; // game ticks between frame advances
    bool loop{true}; // wraps to startFrame when done; false = clamp + signal finish
};

/**
 * @brief Output of the animation systems; the sole input to drawSystem for sprite selection.
 *
 * **Producers:** playerAnimSystem, patrollerAnimSystem, locksterAnimSystem,
 *                bossAnimSystem, explosionAnimSystem — all via tickAnim<>.
 * **Consumers:** drawSystem (spriteIndex selects the horizontal column to blit),
 *                explosionAnimSystem / bossAnimSystem (finishedThisTick triggers destroy).
 *
 * finishedThisTick is set true for exactly one tick when a non-looping clip ends.
 * tickAnim<> clears it to false at the start of every call.
 */
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
        Run,
        Jump,
        ShootIdle,
        ShootRun
    };
    State                        state{State::Idle};
    State                        prev{State::Idle};
    int                          frame{};
    int                          timer{};
    int                          shootHoldTicks{};
    std::array<AnimationClip, 5> clips{};
};

struct PatrollerAnimation
{
    enum class State
    {
        Run,
        Burst
    };
    State                        state{State::Run};
    State                        prev{State::Run};
    int                          frame{};
    int                          timer{};
    std::array<AnimationClip, 2> clips{};
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

struct BossAnimation
{
    enum class State
    {
        IDLE,
        CHARGE_DASH,
        DASH,
        SHOOT,
        DIE,
        BRAKE // animation-only, no matching BossAI state
    };
    State                        state{State::IDLE};
    State                        prev{State::IDLE};
    int                          frame{};
    int                          timer{};
    std::array<AnimationClip, 6> clips{};
};

/**
 * @brief Finite-state machine data for the boss entity.
 *
 * State cycle: IDLE → (alternates) CHARGE_DASH → DASH → IDLE
 *                                   SHOOT → IDLE
 * CHARGE_DASH locks the dash direction (dashRight) at entry so the player has
 * a moment to react. DIE is entered by healthSystem and bossAnimSystem destroys
 * the entity when the death animation's finishedThisTick fires.
 */
struct BossAI
{
    enum class State
    {
        IDLE,
        CHARGE_DASH,
        DASH,
        SHOOT,
        DIE
    };
    State state{State::IDLE};
    int   stateTimer{};     ///< counts down in IDLE / CHARGE_DASH / DASH
    bool  dashRight{};      ///< dash direction, locked in CHARGE_DASH
    bool  nextIsDash{true}; ///< alternates attack selection
    int   shotsFired{};     ///< bullets fired so far this SHOOT
    int   shotTimer{};      ///< ticks until next bullet
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
    bool  prevMoveUp{};
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
    bool  isDead{};
    int   invulnerableTimer{};
};

struct Input
{
    // Preferred storage: Tagged, we only need to know if entity has it
    // so it can react to input events.
};

/**
 * @brief Producer/consumer contract for desired actions this frame.
 *
 * **Producers:** inputSystem (player), aiSystem/bossSystem (enemies).
 * **Consumers:** movementSystem (direction + speed), jumpSystem (moveUp),
 *                shootingSystem (shoot flag).
 * Reset to {} by the AI tick functions at the start of each frame to prevent stale input.
 */
struct Intent
{
    bool moveLeft{};
    bool moveRight{};
    bool moveUp{};
    bool moveDown{};
    bool shoot{};
};

/**
 * @brief One-tick damage request queued by collisionSystem and consumed by damageSystem.
 *
 * **Producer:** collisionSystem on bullet-hit or player-enemy body contact.
 * **Consumer:** damageSystem subtracts amount from Health::points (unless invulnerable),
 *               then clears pending. fromContact and fromFall distinguish the source for
 *               i-frame and respawn logic. Only one pending damage per entity per tick;
 *               later contacts in the same step overwrite earlier ones.
 */
struct DamageIntent
{
    float amount{};
    bool  pending{};
    bool  fromContact{};
    bool  fromFall{};
};

struct Enemy
{
    // Preferred storage: Tagged, only need to know if entity has it.
};

/**
 * @brief General-purpose AI state machine for patrol-type enemies.
 *
 * aiSystem dispatches to tickPatroller or tickLockster based on AI::type each frame.
 * freezeFrames provides a short stun on player damage so enemies pause reacting.
 * spawnX/spawnY store the original position for respawn relocation.
 */
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
        CHASE_SHOOT,
        BURST
    };

    Type  type{};
    State state{};
    float patrolMinX{};
    float patrolMaxX{};
    float detectionRange{};
    int   alertTimer{};
    float spawnX{};
    float spawnY{};
    int   shootCooldown{};
    int   shotsFired{};
    bool  patrollingRight{true};
    float targetX{};
    int   freezeFrames{};
    int   burstTimer{};
    int   burstCooldown{};
    bool  burstFired{};
};

struct Weapon
{
    // Preferred storage: Stack, few entities will be able to shoot
    // and be in the scene at the same time.

    enum Type { Normal, Boss }; // value = column index in shots.png

    Type  type{Normal};
    float damage{};
    float shotSpeed{};
    int   shootCooldown{};
};

struct Projectile
{
    Weapon::Type type{};
    float        damage{};
    bool         fromEnemy{};
};

struct Respawn
{
    float spawnX{};
    float spawnY{};
    float lastCheckpointX{};
    float lastCheckpointY{};
    float maxHp{};
    int   flickerTimer{};
    bool  isRespawning{};
};

// ============= SYSTEMS    =============

/** @brief Polls SDL keyboard state and writes Intent for every entity with the Input tag. */
void inputSystem();

/**
 * @brief Applies Intent to Box2D velocity, syncs MTransform from physics, and kills entities that fall off the map.
 *
 * @param sceneMinY  World Y below which an entity is considered out-of-bounds.
 *                   Players receive lethal DamageIntent; other entities are destroyed immediately.
 */
void movementSystem(float sceneMinY);

/**
 * @brief Advances the player's last-checkpoint record when a new checkpoint X is passed.
 *
 * Iterates checkpoints in world-X order; stores the rightmost one the player has crossed.
 * The stored position is used by respawnSystem on death.
 *
 * @param checkpoints  Player spawn/checkpoint list from the scene's spawn layer.
 */
void checkpointSystem(const std::vector<SpawnPoint>& checkpoints);

/**
 * @brief Handles ground detection, coyote time, jump buffering, and variable-gravity on fall.
 *
 * Ground detection: three raycasts downward (center, left edge, right edge) with a small
 * probe past the sprite bottom. Coyote time lets the player jump briefly after walking off
 * a ledge. Jump buffer fires a queued jump on the first grounded frame after a premature press.
 * Fall gravity is multiplied by PLAYER_JUMP_FALL_FACTOR for a snappier feel.
 */
void jumpSystem();

/** @brief Selects PlayerAnimation::State from movement/jump/shoot context, then calls tickAnim. */
void playerAnimSystem();

/** @brief Ticks the PatrollerAnimation state machine via tickAnim. */
void patrollerAnimSystem();

/**
 * @brief Derives LocksterAnimation state from AI detection range and alertTimer, then calls tickAnim.
 *
 * Reads the player's position each frame to decide Idle / Alert / Charge state.
 */
void locksterAnimSystem();

/**
 * @brief Advances the explosion one-shot animation; destroys the entity when finishedThisTick fires.
 *
 * Deferred destruction (collect then destroy) avoids invalidating the entity iterator mid-loop.
 */
void explosionAnimSystem();

/** @brief Locates the boss entity and calls tickBoss with the current player position. */
void bossSystem();

/**
 * @brief Maps BossAI::State → BossAnimation::State, adds the BRAKE sub-state at the
 *        end of a dash, ticks animation, and destroys the boss when the DIE clip finishes.
 */
void bossAnimSystem();

/**
 * @brief Renders all entities with MTransform + Drawable + RenderFrame + Movement.
 *
 * Converts world positions to screen pixels via transformToFrect. Flips the sprite
 * horizontally when (facingLeft == defaultFacingLeft) — defaultFacingLeft encodes
 * which direction the source art faces. Invulnerable entities flicker (hidden every
 * other 4-tick window).
 *
 * @param ren  SDL renderer to draw to.
 */
void drawSystem(SDL_Renderer* ren);

/**
 * @brief Renders the player's health as a row of heart sprites in screen-space.
 *
 * Full hearts are drawn at full alpha; half hearts show a faded background heart
 * plus a half-wide foreground crop; empty hearts are drawn at reduced alpha.
 *
 * @param ren       SDL renderer.
 * @param heartTex  8×8 heart sprite texture.
 */
void hudSystem(SDL_Renderer* ren, SDL_Texture* heartTex);

/**
 * @brief Fires a projectile when Intent::shoot is true and the weapon cooldown has elapsed.
 *
 * Resets cooldown after firing. Player projectiles use PLAYER_SHOOT_COOLDOWN;
 * enemy projectiles use PATROLLER_SHOOT_COOLDOWN. Triggering also sets
 * PlayerAnimation::shootHoldTicks so the shoot-pose is held for a few frames.
 */
void shootingSystem();

/**
 * @brief Steps the Box2D world, then walks all contact-begin events to queue damage.
 *
 * After the physics step, also updates the camera to follow the player.
 * Contact dispatch rules:
 *  - Bullet vs. world geometry  → destroy bullet (no damage).
 *  - Bullet vs. invulnerable enemy → bounce bullet X velocity, no damage.
 *  - Player bullet vs. enemy    → enemy DamageIntent.
 *  - Enemy bullet vs. player    → player DamageIntent.
 *  - Player body vs. enemy body → player DamageIntent (contact damage).
 * Entity lookup: b2Body_GetUserData returns the bagel entity id stored in createX().
 *
 * @param box  The active Box2D world id.
 */
void collisionSystem(b2WorldId box);

/** @brief Destroys projectiles whose MTransform is outside the camera bounds (plus bullet margin). */
void projectileCullSystem();

/**
 * @brief Consumes pending DamageIntent and applies it to Health.
 *
 * Skips entities whose Health::isInvulnerable is true. On player damage: starts
 * PLAYER_HIT_IFRAMES and freezes all enemies for PLAYER_HIT_FREEZE_FRAMES.
 * Clears DamageIntent::pending after processing regardless of invulnerability.
 */
void damageSystem();

/**
 * @brief Ticks invulnerability timers and handles death for all entities with Health.
 *
 * Death behaviour by entity type:
 *  - Boss       → transitions BossAI to DIE state (bossAnimSystem destroys it later).
 *  - Enemy      → spawns an explosion, destroys Box2D body, queues entity destruction.
 *  - Player     → sets Health::isDead (respawnSystem handles the actual respawn).
 */
void healthSystem();

/**
 * @brief Dispatches per-enemy AI tick (tickPatroller or tickLockster) based on AI::type.
 *
 * Locates the player's position first, then iterates enemies. Enemies with
 * AI::freezeFrames > 0 have their Intent cleared and the counter decremented instead.
 */
void aiSystem();

/**
 * @brief Handles player and enemy respawn when Health::isDead becomes true.
 *
 * Player respawn sequence:
 *  1. On first dead tick: freeze velocity, clear intent, destroy all bullets, start flickerTimer.
 *  2. While flickerTimer > 0: freeze velocity every tick.
 *  3. On flickerTimer expiry: teleport to last checkpoint, restore HP, grant respawn i-frames,
 *     reset all AI enemies to spawn positions, and re-create any missing Locksters.
 *
 * @param world       Box2D world for potential Lockster body re-creation.
 * @param enemySpawns Full enemy spawn list from the scene; used to check/recreate Locksters.
 * @param locksterTex Texture passed to createLockster when recreating missing instances.
 */
void respawnSystem(b2WorldId                      world,
                   const std::vector<SpawnPoint>& enemySpawns,
                   SDL_Texture*                   locksterTex);

// ============= ENTITIES   =============

/**
 * @brief Creates the player entity with a dynamic Box2D body, all physics components,
 *        animation clips, weapons, and respawn data.
 *
 * @note Body userData = entity id (cast to void*). collisionSystem relies on this.
 *       Filter: category = CAT_PLAYER, mask = CAT_WORLD | CAT_ENEMY | CAT_ENEMY_BULLET.
 */
ent_type createPlayer(b2WorldId world, float x, float y, SDL_Texture* tex);

/**
 * @brief Creates a Patroller enemy with a kinematic Box2D body and patrol AI.
 *
 * @param patrolMinX  Left boundary of the patrol range in world units.
 * @param patrolMaxX  Right boundary of the patrol range in world units.
 * @note Body userData = entity id. Filter: category = CAT_ENEMY.
 */
ent_type createPatroller(b2WorldId    world,
                         float        x,
                         float        y,
                         float        patrolMinX,
                         float        patrolMaxX,
                         SDL_Texture* tex);

/**
 * @brief Creates a Lockster enemy with a dynamic bouncy Box2D body.
 *
 * Restitution = 1.0 so the body bounces off walls at equal speed.
 * The Lockster is invulnerable while idle (set by tickLockster).
 * @note Body userData = entity id. Filter: category = CAT_ENEMY.
 */
ent_type createLockster(b2WorldId world, float x, float y, SDL_Texture* tex);

/**
 * @brief Creates the boss entity with a shrunk hitbox and full BossAI state machine.
 *
 * The Box2D hitbox is smaller than the sprite (HITBOX_SHRINK_FACTOR) and offset
 * downward to make dodging easier. Carries BossAI, BossAnimation, and Weapon.
 * @note Body userData = entity id. Filter: category = CAT_ENEMY.
 */
ent_type createBoss(b2WorldId world, float x, float y, SDL_Texture* tex);

/** @brief Creates a static or moving platform (no Box2D body; physics not yet implemented). */
ent_type createPlatform(float x, float y, bool isMoving);

/**
 * @brief Spawns a projectile bullet with a sensor-like dynamic Box2D body.
 *
 * Velocity is set immediately via b2Body_SetLinearVelocity (velX/velY × FPS).
 * gravityScale = 0 so bullets travel in a straight line.
 * Filter category is CAT_PLAYER_BULLET or CAT_ENEMY_BULLET depending on fromEnemy.
 * spriteIndex in RenderFrame is set to the weapon type column in shots.png.
 *
 * @param velX      Horizontal velocity in world units/tick (multiplied by FPS internally).
 * @param velY      Vertical velocity in world units/tick.
 * @param type      Selects bullet size and sprite column.
 * @param damage    Amount to subtract from the target's Health::points.
 * @param fromEnemy Determines filter bits and DamageIntent routing in collisionSystem.
 */
ent_type createProjectile(b2WorldId    world,
                          float        x,
                          float        y,
                          float        velX,
                          float        velY,
                          Weapon::Type type,
                          float        damage,
                          bool         fromEnemy);

/** @brief Spawns a one-shot explosion animation entity at (x, y). Destroyed by explosionAnimSystem. */
ent_type createExplosion(float x, float y);

/** @brief Creates an invisible trigger rectangle (no physics, no render — placeholder). */
ent_type createTrigger(float x, float y, float width, float height);

/** @brief Creates a placeholder item entity (no texture/behavior yet). */
ent_type createItem(float x, float y);

} // namespace megaman
