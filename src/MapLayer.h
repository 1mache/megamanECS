#pragma once

#include "Texture.h"

#include <SDL3/SDL.h>
#include <tmxlite/Map.hpp>

#include <memory>
#include <vector>

class MapLayer final
{
public:
    MapLayer() = default;

    bool create(const tmx::Map&                              map,
                std::uint32_t                                layerIndex,
                const std::vector<std::unique_ptr<Texture>>& textures);

    void draw(SDL_Renderer* renderer) const;

private:
    // one subset per used tileset
    struct Subset final
    {
        std::vector<SDL_Vertex> vertexData;
        SDL_Texture*            texture = nullptr;
    };
    std::vector<Subset> _subsets;
};
