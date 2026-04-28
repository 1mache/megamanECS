#ifndef MEGAMAN_H
#define MEGAMAN_H

#include "SDL3/SDL.h"
#include "bagel.h"
#include <string>

namespace megaman
{ // forward declarations
struct Transform;
struct Movement;
struct Collision;
struct Drawable;
struct Health;
struct Input;
struct Enemy;
struct AI;
struct Weapon;
struct Scene;
struct Sound;
} // namespace megaman

// ============= STORAGE SPECIALIZATIONS =============

template <>
struct bagel::Storage<megaman::Transform> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<megaman::Transform>;
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
struct bagel::Storage<megaman::Scene> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<megaman::Scene>;
};

template <>
struct bagel::Storage<megaman::Sound> final : bagel::NoInstance
{
    using type = bagel::StackStorage<megaman::Sound>;
};

namespace megaman
{
using ent_type = bagel::ent_type;

// ============= COMPONENTS =============
struct Transform
{
    // Preferred storage: Sparse, almost every entity will have a transform,
    // so almost no holes and array will be well utilized.

    // pos and scale can also be a Vector2 or equivalent if exists in SDL. or stay like this.
    float posX{};
    float posY{};
    float scaleX{};
    float scaleY{};
    // degrees or radians but can be converted into another type later
    float rotation{};
};

struct Movement
{
    // Preferred storage: Packed, per frame iteration even though entities
    // with movement will be created and destroyed "frequently" so stack is an option too.

    float mass{};
    float velX{};
    float velY{};
    float accX{}; //acceleration
    float accY{};
};

struct Collision
{
    // Preferred storage: Packed, same reason as Movement since its also physics related.

    // for now: collider box centered on entity position, so only width and height matter.
    // note: possible different shapes of colliders in the future so shape member can be added.
    float width{};
    float height{};
};

struct Drawable
{
    // Preferred storage: Sparse, almost every entity can be drawn to the screen.
    // Holes will be filled quickly with new objects so we can get away with one array.

    SDL_Texture* texture{nullptr};
};

struct Health
{
    // Preferred storage: Stack, relatively few entities will have health and
    // they are created and destroyed a lot.

    int  points{};
    bool isInvulnerable{};
    bool isDead{};
};

struct Input
{
    // Preferred storage: Tagged, we only need to know if entity has it
    // so it can react to input events.
};

struct Enemy
{
    // Preferred storage: Tagged, only need to know if entity has it.
};

struct AI
{
    // Preferred storage: Stack, relatively few entities will have AI
    // and they are likely to be created and destroyed frequently.

    int state{-1}; // or some custom type later
};

struct Weapon
{
    // Preferred storage: Stack, few entities will be able to shoot
    // and be in the scene at the same time, we dont want to waste large array for their possibly large ids.

    int projectileType{-1}; // or some custom type later
};

struct Scene
{
    // Preferred storage: Sparse, probably only one entity of this type with
    // small id value so one short array is good.

    std::string mapFilePath{"res/..."}; // or some way to hold layout data
};

struct Sound
{
    // Preferred storage: Stack, a lot of entities can have sound effects tied to them
    // and a lot of them die and get created frequently.

    int  sound{-1}; // or some way to hold sound data
    bool isPlaying{false};
};

// ============= SYSTEMS    =============

class MovementSystem final : bagel::NoInstance
{
};
// maybe unite these 2 into one PhysicsSystem later
class CollisionSystem final : bagel::NoInstance
{
};

class DrawingSystem final : bagel::NoInstance
{
};

class HealthSystem final : bagel::NoInstance
{
};

class AISystem final : bagel::NoInstance
{
};

class InputSystem final : bagel::NoInstance
{
};

class SoundSystem final : bagel::NoInstance
{
};

class SceneSystem final : bagel::NoInstance
{
};
// ============= ENTITIES   =============

ent_type createPlayer(float x, float y /*or vector2 like type*/, int hp);

ent_type createEnemy(float x, float y, int hp);

ent_type createBoss(float x, float y, int hp);

ent_type createPlatform(float x, float y, bool isMoving);

ent_type createProjectile(float x, float y, float velX, float velY);

ent_type createTrigger(float x, float y, float width, float height);

ent_type createItem(float x, float y);

ent_type createText(float x, float y, const std::string& text);

ent_type createSoundSource(float x, float y, int sound);

ent_type createScene(const std::string& mapFilePath);
} // namespace megaman

#endif // MEGAMAN_H
