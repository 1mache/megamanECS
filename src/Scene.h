/**
 * @file Scene.h
 * @brief Loads and owns all TMX map layers (tile, image, collision, spawn).
 */
#pragma once
#include "CameraData.h"
#include "MapCollisionLayer.h"
#include "MapImageLayer.h"
#include "MapSpawnLayer.h"
#include "MapTileLayer.h"
#include "SpawnPoint.h"
#include "Texture.h"
#include <SDL3/SDL.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace megaman
{
/**
 * @brief Top-level map container: loads all TMX layers and exposes their data to the game.
 *
 * Lifetime: construct with a TMX file path, call load() once, then call
 * attachPhysics() after the Box2D world is created. After that the scene is
 * ready to draw and query each frame.
 */
class Scene
{
public:
    static constexpr const char* SOLID_CLASS  = "solid";
    static constexpr const char* SPAWNS_CLASS = "spawns";

public:
    explicit Scene(std::string filePath) : _filePath(std::move(filePath)) {};

    bool isValid() const
    {
        return _loaded && !_tileLayers.empty() && !_collisionLayers.empty() &&
               _spawnLayer.has_value();
    }

    /**
     * @brief Loads and parses the TMX file, constructing all layer objects.
     *
     * Dispatches each TMX layer to the appropriate processor based on layer type
     * and class ("solid" object layers → collision, "spawns" object layers → spawn).
     * Asserts that exactly one tileset is present. Calls fatalError on failure.
     */
    void load(SDL_Renderer* renderer);

    /**
     * @brief Creates static Box2D bodies for all collision layers.
     *
     * Must be called after the Box2D world is created and before the first simulation step.
     */
    void attachPhysics(b2WorldId worldId);

    /** @brief Draws all image layers then all tile layers in order (back-to-front). */
    void draw(SDL_Renderer* renderer, const CameraData& cam) const;

    WorldBoundsM getBoundsM() const;

    /**
     * @brief Clamps @p cam so the viewport never reveals area outside the map.
     *
     * Computes the legal camera center range as [minX + halfView, maxX − halfView].
     * If the view is wider than the map on an axis the camera is locked to the map
     * center on that axis (prevents over-clamping that would black-bar one side).
     */
    void clampCameraToBounds(CameraData& cam) const;

    const std::vector<SpawnPoint>& getPlayerSpawns() const;
    const std::vector<SpawnPoint>& getEnemySpawns() const;
    const std::vector<SpawnPoint>& getItemSpawns() const;

private:
    /** @brief Creates and stores a MapTileLayer from the given TMX tile layer. */
    void processTileLayer(const tmx::Layer::Ptr& layer,
                          unsigned int           idx,
                          const tmx::Map&        map);
    /** @brief Loads the image texture and creates a MapImageLayer with parallax settings. */
    void processImageLayer(SDL_Renderer*          renderer,
                           const tmx::Layer::Ptr& layer,
                           unsigned int           idx,
                           const tmx::Map&        map);
    /** @brief Creates a MapCollisionLayer from a "solid"-class object layer. */
    void processCollisionLayer(const tmx::Layer::Ptr& layer,
                               unsigned int           idx,
                               const tmx::Map&        map);
    /** @brief Creates the MapSpawnLayer from a "spawns"-class object layer. Only one is supported. */
    void processObjectLayer(const tmx::Layer::Ptr& layer,
                            unsigned int           idx,
                            const tmx::Map&        map);


    std::string                                     _filePath;
    bool                                            _loaded{};
    std::vector<std::unique_ptr<Texture>>           _textures;
    std::vector<std::unique_ptr<MapTileLayer>>      _tileLayers;
    std::vector<std::unique_ptr<MapImageLayer>>     _imageLayers;
    std::vector<std::unique_ptr<MapCollisionLayer>> _collisionLayers;
    std::optional<MapSpawnLayer>                    _spawnLayer;
};
} // namespace megaman
