#pragma once

#include "MTransform.h"

#include <box2d/box2d.h>
#include <tmxlite/Map.hpp>

#include <vector>

namespace megaman
{

class MapCollisionLayer final
{
public:
    MapCollisionLayer() = default;

    bool create(const tmx::Map& map, std::uint32_t layerIndex);

    // create static b2d box bodies for each rect in _rects
    void createBodies(b2WorldId worldId);
    bool isValid() const
    {
        return !_rects.empty();
    }

private:
    // world coords: center + half-extents, Y-up, meters
    std::vector<MTransform> _rects;

    static constexpr const char* DOOR_CLASS         = "door";
    static constexpr const char* DOOR_TRIGGER_CLASS = "door_trigg";
};

} // namespace megaman
