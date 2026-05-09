#pragma once
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace megaman
{
struct MTransform
{
    // *In world coordinates
    // Preferred storage: Sparse, almost every entity will have a transform,
    // so almost no holes and array will be well utilized.

    float x{}, y{}; // center
    float w{}, h{}; // from the center. like box2d
    // degrees or radians but can be converted into another type later
    float rot{};
};

SDL_FRect transformToFrect(const MTransform& t);

constexpr b2Vec2 transformToB2Pos(const MTransform& t)
{
    return b2Vec2{t.x, t.y};
}

constexpr b2Vec2 transformToB2Scale(const MTransform& t)
{
    return b2Vec2{t.w, t.h};
}

constexpr void transformUpdateWithB2Pos(MTransform& t, b2Vec2 pos)
{
    t.x = pos.x;
    t.y = pos.y;
}

} // namespace megaman
