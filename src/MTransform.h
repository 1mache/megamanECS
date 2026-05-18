/**
 * @file MTransform.h
 * @brief World-space transform and world↔screen conversion helpers.
 *
 * MTransform stores positions in meters (Y-up). All rendering converts via
 * worldToScreenPoint / worldToScreenSize before passing to SDL.
 */
#pragma once
#include "CameraData.h"
#include "GlobalData.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace megaman
{
/** @brief World-space AABB transform: center (x,y), half-extents (w,h), rotation. */
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

/**
 * @brief Converts a world point to SDL screen coordinates.
 *
 * Maps meters (Y-up, camera-relative) to pixels (Y-down, window-relative).
 * Formula: screenX = winCenterX + (worldX − camX) × PTM × scale
 *          screenY = winCenterY − (worldY − camY) × PTM × scale
 * Used by every rendering call in drawSystem and MapTileLayer::draw.
 *
 * @param worldPos  Position in world units (meters, Y-up).
 * @param cam       Current camera center in world units.
 * @return SDL_FPoint in screen pixels (Y-down, origin top-left).
 */
SDL_FPoint worldToScreenPoint(SDL_FPoint worldPos, const CameraData& cam);

/** @brief Converts a world-unit length to screen pixels (PTM × SCALE_FACTOR). */
constexpr float worldToScreenSize(float worldSize)
{
    return worldSize * GlobalData::PTM * GlobalData::SCALE_FACTOR;
}

/** @brief Converts a screen-pixel length to world units (inverse of worldToScreenSize). */
constexpr float screenToWorldSize(float pixelSize)
{
    return pixelSize / (GlobalData::PTM * GlobalData::SCALE_FACTOR);
}

/** @brief Converts an MTransform to an SDL_FRect in screen pixels for rendering. */
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
