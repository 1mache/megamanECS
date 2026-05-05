#include "MapLayer.h"

#include <tmxlite/TileLayer.hpp>

#include <cassert>

bool MapLayer::create(const tmx::Map& map, std::uint32_t layerIndex,
                      const std::vector<std::unique_ptr<Texture>>& textures)
{
    const auto& layers      = map.getLayers();
    assert(layers[layerIndex]->getType() == tmx::Layer::Type::Tile);

    const auto& layer       = layers[layerIndex]->getLayerAs<tmx::TileLayer>();
    const auto  mapSize     = map.getTileCount();
    const auto  mapTileSize = map.getTileSize();
    const auto& tileSets    = map.getTilesets();

    const auto tint = layer.getTintColour();
    const SDL_FColor vertColour =
    {
        tint.r / 255.f,
        tint.g / 255.f,
        tint.b / 255.f,
        tint.a / 255.f
    };

    for (auto i = 0u; i < tileSets.size(); ++i)
    {
        const auto& ts      = tileSets[i];
        const auto& tileIDs = layer.getTiles();

        const auto texSize    = textures[i]->getSize();
        const int  tileCountX = texSize.x / static_cast<int>(mapTileSize.x);

        const float uNorm = static_cast<float>(mapTileSize.x) / static_cast<float>(texSize.x);
        const float vNorm = static_cast<float>(mapTileSize.y) / static_cast<float>(texSize.y);

        std::vector<SDL_Vertex> verts;
        for (auto y = 0u; y < mapSize.y; ++y)
        {
            for (auto x = 0u; x < mapSize.x; ++x)
            {
                const auto idx = y * mapSize.x + x;
                if (idx >= tileIDs.size()
                    || tileIDs[idx].ID < ts.getFirstGID()
                    || tileIDs[idx].ID >= ts.getFirstGID() + ts.getTileCount())
                {
                    continue;
                }

                const auto idIndex = tileIDs[idx].ID - ts.getFirstGID();
                float u = static_cast<float>(idIndex % static_cast<std::uint32_t>(tileCountX))
                          * static_cast<float>(mapTileSize.x) / static_cast<float>(texSize.x);
                float v = static_cast<float>(idIndex / static_cast<std::uint32_t>(tileCountX))
                          * static_cast<float>(mapTileSize.y) / static_cast<float>(texSize.y);

                const float px = static_cast<float>(x) * static_cast<float>(mapTileSize.x);
                const float py = static_cast<float>(y) * static_cast<float>(mapTileSize.y);
                const float tw = static_cast<float>(mapTileSize.x);
                const float th = static_cast<float>(mapTileSize.y);

                verts.push_back({ { px,      py      }, vertColour, { u,         v         } });
                verts.push_back({ { px + tw, py      }, vertColour, { u + uNorm, v         } });
                verts.push_back({ { px,      py + th }, vertColour, { u,         v + vNorm } });

                verts.push_back({ { px,      py + th }, vertColour, { u,         v + vNorm } });
                verts.push_back({ { px + tw, py      }, vertColour, { u + uNorm, v         } });
                verts.push_back({ { px + tw, py + th }, vertColour, { u + uNorm, v + vNorm } });
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

void MapLayer::draw(SDL_Renderer* renderer) const
{
    assert(renderer);
    for (const auto& s : _subsets)
    {
        SDL_RenderGeometry(renderer, s.texture,
            s.vertexData.data(), static_cast<int>(s.vertexData.size()),
            nullptr, 0);
    }
}
