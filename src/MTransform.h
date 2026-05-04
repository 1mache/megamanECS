#pragma once
#include "GlobalData.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace megaman
{
struct Transform
{
    // *In world coordinates
    // Preferred storage: Sparse, almost every entity will have a transform,
    // so almost no holes and array will be well utilized.

    float x{}, y{}; // center
    float w{}, h{}; // from the center. like box2d
    // degrees or radians but can be converted into another type later
    float rot{};
};

SDL_FRect transformToFrect(const Transform& t)
{
    auto ptm = GlobalData::PTM;
    auto winh = GlobalData::winH();
    // invert y and convert to pixels
    return SDL_FRect{.x = (t.x - t.w) * ptm, .y = winh - ((t.y + t.h) * ptm), .w = t.w * 2 * ptm, .h = t.h * 2 * ptm};
}

constexpr b2Vec2 transformToB2Pos(const Transform& t)
{
    return b2Vec2{t.x, t.y};
}

constexpr b2Vec2 transformToB2Scale(const Transform& t)
{
    return b2Vec2{t.w, t.h};
}

constexpr void transformUpdateWithB2Pos(Transform& t, b2Vec2 pos)
{
    t.x = pos.x;
    t.y = pos.y;
}

} // namespace megaman
