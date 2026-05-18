/**
 * @file MapSpawnLayer.h
 * @brief Parses the TMX object layer that defines player checkpoints and enemy/item spawns.
 */
#pragma once

#include "SpawnPoint.h"

#include <tmxlite/Map.hpp>

#include <vector>

namespace megaman
{

/**
 * @brief Holds all spawn/checkpoint positions extracted from the map's spawn object layer.
 *
 * Player checkpoints are ordered by X position so the rightmost visited one
 * becomes the active respawn point. Enemy spawns carry type-specific metadata
 * (e.g., patrol destination for Ptrol entries).
 */
class MapSpawnLayer final
{
public:
    MapSpawnLayer() = default;

    /**
     * @brief Parses all point objects from layer @p layerIndex, classifying them by TMX class name.
     *
     * Patrol enemies require a matching "ptrold" (PtrolDest) object of the same name
     * to set their patrol range; if none is found the spawn X is used as the destination.
     * Objects that are not point-shaped are skipped.
     *
     * @return true if at least one player checkpoint was parsed.
     */
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
