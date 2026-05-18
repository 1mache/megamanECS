/**
 * @file MapCollisionLayer.h
 * @brief Parses TMX object-layer rectangles into static Box2D collision bodies.
 */
#pragma once

#include "MTransform.h"

#include <box2d/box2d.h>
#include <tmxlite/Map.hpp>

#include <vector>

namespace megaman
{

/**
 * @brief Two-phase loader for solid map geometry.
 *
 * Call create() at load time to build the list of rectangles from the TMX file,
 * then call createBodies() after the Box2D world exists to instantiate static bodies.
 */
class MapCollisionLayer final
{
public:
    MapCollisionLayer() = default;

    /**
     * @brief Parses rectangle objects from TMX layer @p layerIndex into world-space half-extents.
     *
     * TMX pixel-space Y-down is flipped to world Y-up during parsing.
     *
     * @return true if at least one rectangle was parsed.
     */
    bool create(const tmx::Map& map, std::uint32_t layerIndex);

    /**
     * @brief Creates a static Box2D body for every rectangle collected by create().
     *
     * All bodies use CAT_WORLD category and collide with players, enemies, and bullets.
     * Body userData is set to -1 (sentinel: not an ECS entity).
     *
     * @pre create() must have returned true.
     */
    void createBodies(b2WorldId worldId);
    bool isValid() const
    {
        return !_rects.empty();
    }

private:
    // world coords: center + half-extents, Y-up, meters
    std::vector<MTransform> _rects;
};

} // namespace megaman
