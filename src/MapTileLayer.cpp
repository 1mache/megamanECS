#include "MapTileLayer.h"

#include <tmxlite/TileLayer.hpp>

#include <cassert>

bool MapTileLayer::create(const tmx::Map& map, std::uint32_t layerIndex,
                          const std::vector<std::unique_ptr<Texture>>& textures)
{
    const auto& layers = map.getLayers();
    // we are loading a tile layer. not object or image
    assert(layers[layerIndex]->getType() == tmx::Layer::Type::Tile);

    const auto& layer = layers[layerIndex]->getLayerAs<tmx::TileLayer>();
    const auto  mapSize = map.getTileCount();
    const auto  mapTileSize = map.getTileSize();
    const auto& tileSets = map.getTilesets();

    const auto       tint = layer.getTintColour();
    const SDL_FColor vertColour = {tint.r / 255.f, tint.g / 255.f, tint.b / 255.f, tint.a / 255.f};

    for (auto i = 0u; i < tileSets.size(); ++i)
    {
        const auto& ts = tileSets[i];
        const auto& tileIDs = layer.getTiles();

        const auto texSize = textures[i]->getSize();
        const int  tileCountX = texSize.x / static_cast<int>(mapTileSize.x);

        // UV space: texture coordinates. (0,0) = top-left, (1,1) = bottom-right of texture

        // norms are size of 1 tile in UV space
        const float uNorm = static_cast<float>(mapTileSize.x) / static_cast<float>(texSize.x);
        const float vNorm = static_cast<float>(mapTileSize.y) / static_cast<float>(texSize.y);

        std::vector<SDL_Vertex> verts;
        for (auto y = 0u; y < mapSize.y; ++y)
        {
            for (auto x = 0u; x < mapSize.x; ++x)
            {
                const auto idx = y * mapSize.x + x;
                // skip tiles whose GID doesnt belong to current tileset
                if (idx >= tileIDs.size() || tileIDs[idx].ID < ts.getFirstGID() ||
                    tileIDs[idx].ID >= ts.getFirstGID() + ts.getTileCount())
                {
                    continue;
                }

                const auto localId = tileIDs[idx].ID - ts.getFirstGID();
                // calculate coordinates of tile in uv space
                float u = static_cast<float>(localId % static_cast<std::uint32_t>(tileCountX)) *
                          static_cast<float>(mapTileSize.x) / static_cast<float>(texSize.x);
                float v = static_cast<float>(localId / static_cast<std::uint32_t>(tileCountX)) *
                          static_cast<float>(mapTileSize.y) / static_cast<float>(texSize.y);

                // pixel coordinates
                const float px = static_cast<float>(x) * static_cast<float>(mapTileSize.x);
                const float py = static_cast<float>(y) * static_cast<float>(mapTileSize.y);
                // tile dims
                const float tw = static_cast<float>(mapTileSize.x);
                const float th = static_cast<float>(mapTileSize.y);

                // calculate tile vertex
                // Triangle1:
                // TL
                verts.push_back({{px, py}, vertColour, {u, v}});
                // TR
                verts.push_back({{px + tw, py}, vertColour, {u + uNorm, v}});
                // BL
                verts.push_back({{px, py + th}, vertColour, {u, v + vNorm}});
                // Triangle2:
                // BL
                verts.push_back({{px, py + th}, vertColour, {u, v + vNorm}});
                // TR
                verts.push_back({{px + tw, py}, vertColour, {u + uNorm, v}});
                // BR
                verts.push_back({{px + tw, py + th}, vertColour, {u + uNorm, v + vNorm}});
            }
        }

        if (!verts.empty())
        {
            _subsets.emplace_back();
            _subsets.back().texture = *textures[i];
            _subsets.back().vertexData.swap(verts);
        }
    }

    return true;
}

void MapTileLayer::draw(SDL_Renderer* renderer) const
{
    assert(renderer);
    for (const auto& s : _subsets)
    {
        // draw all verts using that texture
        SDL_RenderGeometry(renderer, s.texture, s.vertexData.data(), static_cast<int>(s.vertexData.size()), nullptr, 0);
    }
}
