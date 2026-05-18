/**
 * @file WorldBoundsM.h
 * @brief Axis-aligned bounding rectangle in world units (meters, Y-up).
 */
#pragma once
namespace megaman
{
/** @brief World-space AABB; used for camera clamping, culling, and scene bounds. */
struct WorldBoundsM
{
    float minX{}, maxX{}, minY{}, maxY{};
};
} // namespace megaman