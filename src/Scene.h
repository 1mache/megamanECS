#pragma once
#include "CameraData.h"
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
    void draw(SDL_Renderer* renderer, const CameraData& cam) const;

    WorldBoundsM getBoundsM() const;
    // Clamps cam so view never shows outside map. If view bigger than map on
    // an axis, locks cam to map center on that axis.
    void clampCameraToBounds(CameraData& cam) const;

private:
    std::string                                _filePath;
    bool                                       _loaded{};
    std::vector<std::unique_ptr<Texture>>      _textures;
    std::vector<std::unique_ptr<MapTileLayer>> _tileLayers;
};
} // namespace megaman
