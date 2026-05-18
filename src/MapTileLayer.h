/**
 * @file MapTileLayer.h
 * @brief Pre-bakes a TMX tile layer into a GPU-ready vertex/index buffer for fast rendering.
 */
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
/**
 * @brief Immutable tile layer: parsed once at load time, drawn every frame via SDL_RenderGeometry.
 *
 * Vertices are stored in world units; draw() transforms them to screen pixels
 * each frame using worldToScreenPoint so camera movement is handled automatically.
 */
class MapTileLayer final
{
public:
    MapTileLayer() = default;

    /**
     * @brief Parses all tiles in layer @p layerIndex and builds vertex/index arrays.
     *
     * For each non-empty tile: resolves the GID to a tileset-local ID, calculates
     * UV coordinates from the tileset texture atlas, and emits 4 vertices + 6 indices
     * (two triangles). Pixel-space Y-down is flipped to world Y-up. Tiles outside
     * the expected tileset GID range are skipped.
     *
     * @return true if at least one vertex was emitted.
     */
    bool create(const tmx::Map&                 map,
                std::uint32_t                   layerIndex,
                const std::unique_ptr<Texture>& texture);

    /**
     * @brief Transforms all world-space vertices to screen pixels and submits to SDL_RenderGeometry.
     *
     * Vertex positions are re-projected each frame (no separate culling; all tiles in the layer
     * are submitted). The tileset texture is used as the geometry texture.
     */
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