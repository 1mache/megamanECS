#pragma once

#include "CameraData.h"
#include "MTransform.h"
#include "Texture.h"

#include <SDL3/SDL.h>
#include <tmxlite/Map.hpp>

namespace megaman
{
class MapImageLayer final
{
public:
    MapImageLayer() = default;

    bool create(const tmx::Map& map,
                std::uint32_t   layerIndex,
                Texture*        texture);
    void draw(SDL_Renderer* renderer, const CameraData& cam) const;

    SDL_FPoint getParallax() const
    {
        return _parallax;
    }

    void setParallax(SDL_FPoint parallax) { _parallax = parallax; }

private:
    SDL_Texture* _texture = nullptr; // non-owning
    MTransform   _transform{};       // bottom-left world pos (x,y), full size (w,h)
    float        _mapWM{};
    float        _mapHM{};
    bool         _repeatX{};
    bool         _repeatY{};
    SDL_FPoint   _parallax{1.f, 1.f};
    SDL_FColor   _tint{1.f, 1.f, 1.f, 1.f};
};
} // namespace megaman
