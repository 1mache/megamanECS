#pragma once
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
    explicit Scene(std::string filePath) : _filePath(std::move(filePath)) {};

    bool isValid() const
    {
        return _loaded && !_tileLayers.empty();
    }

    void load(SDL_Renderer* renderer);
    void draw(SDL_Renderer* renderer, SDL_FPoint camOffset) const;

private:
    std::string                                _filePath;
    bool                                       _loaded{};
    std::vector<std::unique_ptr<Texture>>      _textures;
    std::vector<std::unique_ptr<MapTileLayer>> _tileLayers;
};
} // namespace megaman
