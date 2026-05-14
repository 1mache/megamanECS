#pragma once

#include "SpawnPoint.h"

#include <tmxlite/Map.hpp>

#include <vector>

namespace megaman
{

class MapSpawnLayer final
{
public:
    MapSpawnLayer() = default;

    bool create(const tmx::Map& map, std::uint32_t layerIndex);

    const std::vector<SpawnPoint>& getPlayerCheckpoints() const
    {
        return _playerCheckpoints;
    }

    const std::vector<SpawnPoint>& getEnemiesSpawnPos() const
    {
        return _enemySpawns;
    }

    const std::vector<SpawnPoint>& getItemsSpawnPos() const
    {
        return _itemSpawns;
    }

    bool isValid() const
    {
        return !_playerCheckpoints.empty();
    }

private:
    std::vector<SpawnPoint> _playerCheckpoints;
    std::vector<SpawnPoint> _enemySpawns;
    std::vector<SpawnPoint> _itemSpawns;
};

} // namespace megaman
