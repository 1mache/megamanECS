#pragma once

#include "CameraData.h"
#include "Texture.h"

#include <SDL3/SDL.h>
#include <tmxlite/Map.hpp>

#include <memory>
#include <vector>

class MapTileLayer final
{
public:
    MapTileLayer() = default;

    bool create(const tmx::Map&                              map,
                std::uint32_t                                layerIndex,
                const std::vector<std::unique_ptr<Texture>>& textures);

    void draw(SDL_Renderer* renderer, const megaman::CameraData& cam) const;

    std::string getClassName() const
    {
        return _className;
    }

private:
    // one subset per used tileset
    struct TileSubset final
    {
        std::vector<SDL_Vertex> vertexData;
        SDL_Texture*            texture = nullptr;
    };

    std::vector<TileSubset> _subsets;
    std::string             _className;

    float _minX{};
    float _maxX{};
    float _minY{};
    float _maxY{};
};
