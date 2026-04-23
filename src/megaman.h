#ifndef MEGAMAN_H
#define MEGAMAN_H

#include "SDL3/SDL.h"
#include "bagel.h"
#include <string>

namespace megaman
{
// ============= COMPONENTS =============
struct Transform
{
    // Preferred storage: Packed, almost every entity will have a transform,
    // and its used by many systems every frame.

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
    float accX{};
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
    // Preferred storage: Packed, almost every entity can be drawn to the screen.
    // There wont be many deletions relative to the number of entities
    // and we want quick iteration for rendering

    SDL_Texture *texture{nullptr};
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

    int state{}; // or some custom type later
};

struct Weapon
{
    // Preferred storage: Stack, few entities will be able to shoot
    // and be in the scene at the same time, we dont want to waste large array for their possibly large ids.

    int projectileType{}; // or some custom type later
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

    int sound{}; // or some way to hold sound data
};

// ============= SYSTEMS    =============

// ============= ENTITIES   =============

} // namespace megaman

#endif // MEGAMAN_H
