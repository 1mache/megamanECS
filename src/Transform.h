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

    float x{}, y{};
    float w{}, h{};
    // degrees or radians but can be converted into another type later
    float rotation{};
};

constexpr SDL_FRect transform2Frect(const Transform& t)
{
    auto ptm = GlobalData::PTM;
    auto winh = GlobalData::winH();
    // invert y and convert to pixels
    return SDL_FRect{.x = t.x * ptm,
                     .y = winh - (t.y * ptm),
                     .w = t.w * ptm,
                     .h = t.h * ptm};
}

constexpr b2Vec2 transformGetb2Pos(const Transform& t)
{
    return b2Vec2{t.x, t.y};
}

constexpr b2Vec2 transformGetb2Scale(const Transform& t)
{
    return b2Vec2{t.w, t.h};
}

} // namespace megaman
