#include "MapSpawnLayer.h"

#include "GlobalData.h"

#include <tmxlite/ObjectGroup.hpp>

#include "Utils.h"
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace megaman
{

bool MapSpawnLayer::create(const tmx::Map& map, std::uint32_t layerIndex)
{
    const auto& layers = map.getLayers();
    massert(layerIndex < layers.size(), "Layer index out of bounds");
    massert(layers[layerIndex]->getType() == tmx::Layer::Type::Object,
            "Layer index does not point to an object layer");

    const auto& group = layers[layerIndex]->getLayerAs<tmx::ObjectGroup>();

    constexpr float invPTM = 1.f / GlobalData::PTM;
    const float     mapHM =
        static_cast<float>(map.getTileCount().y * map.getTileSize().y) * invPTM;

    std::unordered_map<std::string, float> patrolDests;
    for (const auto& obj : group.getObjects())
    {
        if (obj.getClass() != spawnTypeToString(SpawnPoint::Type::PtrolDest))
            continue;
        if (obj.getShape() != tmx::Object::Shape::Point)
            continue;

        patrolDests[obj.getName()] = obj.getPosition().x * invPTM;
    }
    // we collected dests. now construct spawn points

    for (const auto& obj : group.getObjects())
    {
        std::string_view objClass = obj.getClass();

        auto shape = obj.getShape();
        // only support point objects here
        if (shape != tmx::Object::Shape::Point)
            continue;

        auto  pos = obj.getPosition();
        float wx  = pos.x * invPTM;
        float wy  = mapHM - pos.y * invPTM;

        SpawnPoint sp{.x = wx, .y = wy};

        if (objClass == spawnTypeToString(SpawnPoint::Type::Player))
        {
            sp.type = SpawnPoint::Type::Player;
            _playerCheckpoints.push_back(sp);
        }
        else if (objClass == spawnTypeToString(SpawnPoint::Type::Ptrol))
        {
            sp.type = SpawnPoint::Type::Ptrol;
            auto it = patrolDests.find(obj.getName());
            // if no patrol dest with the same name is found, just use the spawn point's x as the dest
            sp.patrolDestX = (it != patrolDests.end()) ? it->second : wx;
            _enemySpawns.push_back(sp);
        }
        else if (objClass == spawnTypeToString(SpawnPoint::Type::Lockster))
        {
            sp.type = SpawnPoint::Type::Lockster;
            _enemySpawns.push_back(sp);
        }
        else if (objClass == spawnTypeToString(SpawnPoint::Type::Boss))
        {
            sp.type = SpawnPoint::Type::Boss;
            _enemySpawns.push_back(sp);
        }
    }
    return isValid();
}

} // namespace megaman
