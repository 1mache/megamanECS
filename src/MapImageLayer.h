/**
 * @file MapImageLayer.h
 * @brief Renders a full-image TMX layer with parallax scrolling and optional tiling.
 */
#pragma once

#include "CameraData.h"
#include "MTransform.h"
#include "Texture.h"

#include <SDL3/SDL.h>
#include <tmxlite/Map.hpp>

namespace megaman
{
/**
 * @brief Background/foreground image layer loaded from a TMX Image layer.
 *
 * Supports Tiled's parallax model (parallax factor + map parallax origin),
 * optional horizontal/vertical tiling, and per-layer tint colour.
 */
class MapImageLayer final
{
public:
    MapImageLayer() = default;

    /**
     * @brief Reads image path, offset, parallax factor, tint, and repeat flags from the TMX layer.
     *
     * The image transform is stored as bottom-left + full size (not center + half-extents)
     * because the image is not an ECS entity and doesn't interact with Box2D.
     *
     * @return true if the texture pointer is valid after setup.
     */
    bool create(const tmx::Map& map, std::uint32_t layerIndex, Texture* texture);

    /**
     * @brief Renders the image layer to the screen, applying parallax and tiling.
     *
     * A virtual parallax camera is derived from the real camera and the layer's
     * parallax factor relative to the map's parallax origin. Tiled images are
     * iterated in X and Y to cover the viewport.
     */
    void draw(SDL_Renderer* renderer, const CameraData& cam) const;

    bool isValid() const
    {
        return _texture != nullptr;
    }

    SDL_FPoint getParallax() const
    {
        return _parallax;
    }

    void setParallax(SDL_FPoint parallax)
    {
        _parallax = parallax;
    }

private:
    SDL_Texture* _texture = nullptr; // non-owning
    MTransform   _transform{};       // bottom-left world pos (x,y), full size (w,h)
    float        _mapWM{};
    float        _mapHM{};
    bool         _repeatX{};
    bool         _repeatY{};
    SDL_FPoint   _parallax{1.f, 1.f};
    SDL_FPoint   _parallaxRef{}; // reference point for parallax.
    SDL_FColor   _tint{1.f, 1.f, 1.f, 1.f};
};
} // namespace megaman
