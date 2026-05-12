#pragma once

#include "CameraData.h"
#include "MTransform.h"
#include "Texture.h"
#include "WorldBoundsM.h"

#include <SDL3/SDL.h>
#include <tmxlite/Map.hpp>

#include <memory>
#include <vector>

namespace megaman
{
class MapTileLayer final
{
public:
    MapTileLayer() = default;

    bool create(const tmx::Map&                 map,
                std::uint32_t                   layerIndex,
                const std::unique_ptr<Texture>& texture);

    void draw(SDL_Renderer* renderer, const CameraData& cam) const;

    std::string getClassName() const
    {
        return _className;
    }

    WorldBoundsM getBoundsM() const
    {
        return {_minX, _maxX, _minY, _maxY};
    }

    bool isValid() const
    {
        return _texture && !_vertexData.empty();
    }

private:
    SDL_Texture*            _texture = nullptr;
    std::vector<SDL_Vertex> _vertexData;
    std::vector<int>        _indices;
    std::vector<MTransform> _colliders;
    std::string             _className;

    float _minX{};
    float _maxX{};
    float _minY{};
    float _maxY{};
};
} // namespace megaman