#pragma once
#include "MapTileLayer.h"
#include "Texture.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <string>
#include <tmxlite/Map.hpp>
#include <vector>

namespace megaman
{
class Scene
{
public:
    Scene(std::string_view filePath) : _filePath{filePath}
    {
    }

    void load(SDL_Renderer* renderer);

    bool isValid()
    {
        return !renderLayers.empty();
    }

    void draw(SDL_Renderer* renderer, SDL_FPoint camOffset);

private:
    const std::string                          _filePath;
    std::vector<std::unique_ptr<Texture>>      textures;
    std::vector<std::unique_ptr<MapTileLayer>> renderLayers;
};
} //namespace megaman