#pragma once

#include "MTransform.h"
#include "SDL3/SDL.h"
#include "bagel.h"
#include <box2d/box2d.h>
#include <string>

namespace megaman
{ // forward declarations
    struct Animation;
    struct Movement;
    struct Collision;
    struct Drawable;
    struct Health;
    struct Input;
    struct Intent;
    struct DamageIntent;
    struct Enemy;
    struct AI;
    struct Weapon;
    struct Sound;
    struct Projectile;
} // namespace megaman

// ============= STORAGE SPECIALIZATIONS =============

template <>
struct bagel::Storage<megaman::Animation> final : bagel::NoInstance
{
    using type = bagel::PackedStorage<megaman::Animation>;
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
struct bagel::Storage<megaman::Sound> final : bagel::NoInstance
{
    using type = bagel::StackStorage<megaman::Sound>;
};

template <>
struct bagel::Storage<megaman::Projectile> final : bagel::NoInstance
{
    using type = bagel::StackStorage<megaman::Projectile>;
};

namespace megaman
{
    using ent_type = bagel::ent_type;

    // ============= COMPONENTS =============

    struct Animation
    {
        enum State
        {
            IDLE,
            RUN,
            JUMP
        };
        State state{IDLE};
        int currentFrame{};
        int frameTimer{};
    };

    struct Movement
    {
        // Preferred storage: Packed, per frame iteration even though entities
        // with movement will be created and destroyed "frequently" so stack is an option too.

        float mass{};
        float velX{};
        float velY{};
        float accX{};
        float accY{};
        b2BodyId bodyId{};
        bool facingLeft{};
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

        SDL_Texture *texture{nullptr};
        float spriteW{};
        float spriteH{};
        float drawScale{1.f};
        int idleStart{};
        int idleCount{};
        int runStart{};
        int runCount{};
        int jumpStart{};
        int jumpCount{};
        bool defaultFacingLeft{true};
    };

    struct Health
    {
        // Preferred storage: Stack, relatively few entities will have health and
        // they are created and destroyed a lot.

        float points{};
        bool isInvulnerable{};
        bool isContactInvulnerable{};
        bool isDead{};
        int invulnerableTimer{};
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
        float speed{};
    };

    struct DamageIntent
    {
        // Queued damage written by CollisionSystem, consumed by DamageSystem.
        float amount{};
        bool pending{};
        bool fromContact{};
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

        Type type{};
        State state{};
        float patrolMinX{};
        float patrolMaxX{};
        float detectionRange{};
        float speed{};
        int alertTimer{};
        float chargeSpeed{};
        float spawnX{};
        float spawnY{};
        int shootCooldown{};
        int shotsFired{};
        bool patrollingRight{true};
        float targetX{};
    };

    struct Weapon
    {
        // Preferred storage: Stack, few entities will be able to shoot
        // and be in the scene at the same time.

        int projectileType{-1};
        int shootCooldown{};
    };

    struct Sound
    {
        // Preferred storage: Stack, a lot of entities can have sound effects tied to them
        // and a lot of them die and get created frequently.

        int sound{-1};
        bool isPlaying{false};
    };

    struct Projectile
    {
        bool fromEnemy{};
    };

    // ============= SYSTEMS    =============

    class InputSystem final : bagel::NoInstance
    {
    public:
        static void run();
    };

    class MovementSystem final : bagel::NoInstance
    {
    public:
        static void run();
    };

    class AnimationSystem final : bagel::NoInstance
    {
    public:
        static void run();
    };

    class DrawingSystem final : bagel::NoInstance
    {
    public:
        static void run(SDL_Renderer *ren);
    };

    class ShootingSystem final : bagel::NoInstance
    {
    public:
        static void run();
    };

    class CollisionSystem final : bagel::NoInstance
    {
    public:
        static void run(b2WorldId box);
    };

    class DamageSystem final : bagel::NoInstance
    {
    public:
        static void run();
    };

    class HealthSystem final : bagel::NoInstance
    {
    public:
        static void run();
    };

    class AISystem final : bagel::NoInstance
    {
    public:
        static void run();
    };

    class SoundSystem final : bagel::NoInstance
    {
    public:
        static void run();
    };

    // ============= ENTITIES   =============

    ent_type createPlayer(b2WorldId world, float x, float y, int hp);

    ent_type createPatroller(b2WorldId world, float x, float y, float hp, float patrolMinX, float patrolMaxX, float detectionRange, float speed);

    ent_type createLockster(b2WorldId world, float x, float y, float hp, float detectionRange, float chargeSpeed);

    ent_type createBoss(float x, float y, float hp);

    ent_type createPlatform(float x, float y, bool isMoving);

    ent_type createProjectile(float x, float y, float velX, float velY, bool fromEnemy);

    ent_type createTrigger(float x, float y, float width, float height);

    ent_type createItem(float x, float y);

    ent_type createText(float x, float y, const std::string &text);

    ent_type createSoundSource(float x, float y, int sound);
} // namespace megaman
