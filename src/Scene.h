#pragma once
#include "CameraData.h"
#include "MapCollisionLayer.h"
#include "MapImageLayer.h"
#include "MapTileLayer.h"
#include "Texture.h"
#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <vector>

namespace megaman
{
class Scene
{
public:
    constexpr static const char* SOLID_CLASS = "solid";

public:
    explicit Scene(std::string filePath) : _filePath(std::move(filePath)) {};

    bool isValid() const
    {
        return _loaded && !_tileLayers.empty();
    }

    void load(SDL_Renderer* renderer);
    void attachPhysics(b2WorldId worldId);
    void draw(SDL_Renderer* renderer, const CameraData& cam) const;

    WorldBoundsM getBoundsM() const;
    // Clamps cam so view never shows outside map. If view bigger than map on
    // an axis, locks cam to map center on that axis.
    void clampCameraToBounds(CameraData& cam) const;

private:
    void processTileLayer(const tmx::Layer::Ptr& layer,
                          unsigned int           idx,
                          const tmx::Map&        map);
    void processImageLayer(SDL_Renderer*          renderer,
                           const tmx::Layer::Ptr& layer,
                           unsigned int           idx,
                           const tmx::Map&        map);
    void processCollisionLayer(const tmx::Layer::Ptr& layer,
                               unsigned int           idx,
                               const tmx::Map&        map);


    std::string                                     _filePath;
    bool                                            _loaded{};
    std::vector<std::unique_ptr<Texture>>           _textures;
    std::vector<std::unique_ptr<MapTileLayer>>      _tileLayers;
    std::vector<std::unique_ptr<MapImageLayer>>     _imageLayers;
    std::vector<std::unique_ptr<MapCollisionLayer>> _collisionLayers;
};
} // namespace megaman
