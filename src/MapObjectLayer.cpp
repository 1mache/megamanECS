#include "MapObjectLayer.h"

#include "GlobalData.h"

#include <tmxlite/ObjectGroup.hpp>

#include <cassert>
#include <iostream>
#include <string_view>

namespace megaman
{

bool MapObjectLayer::create(const tmx::Map& map, std::uint32_t layerIndex)
{
    const auto& layers = map.getLayers();
    assert(layerIndex < layers.size());
    assert(layers[layerIndex]->getType() == tmx::Layer::Type::Object &&
           "Layer index does not point to an object layer");

    const auto& group = layers[layerIndex]->getLayerAs<tmx::ObjectGroup>();

    constexpr float invPTM = 1.f / GlobalData::PTM;
    const float     mapHM =
        static_cast<float>(map.getTileCount().y * map.getTileSize().y) * invPTM;

    for (const auto& obj : group.getObjects())
    {
        std::string_view objClass = obj.getClass();

        float wx{};
        float wy{};

        auto shape = obj.getShape();
        if (shape == tmx::Object::Shape::Point)
        {
            auto pos = obj.getPosition();
            wx = pos.x * invPTM;
            wy = mapHM - pos.y * invPTM;
        }
        else // only support point objects here
        {
            continue;
        }

        const SpawnPoint sp{.x = wx, .y = wy};

        if (objClass == PLAYER_CLASS)
        {
            _playerCheckpoints.push_back(sp);
        }
        else if (objClass == ENEMY_CLASS)
        {
            _enemySpawns.push_back(sp);
        }
        else if (objClass == ITEM_CLASS)
        {
            _itemSpawns.push_back(sp);
        }
    }

    return isValid();
}

} // namespace megaman
