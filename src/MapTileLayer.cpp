#include "MapTileLayer.h"

#include "GlobalData.h"
#include "MTransform.h"

#include <tmxlite/TileLayer.hpp>

#include <cassert>
namespace megaman
{
bool MapTileLayer::create(const tmx::Map& map,
                          std::uint32_t   layerIndex,
                          const std::vector<std::unique_ptr<Texture>>& textures)
{
    const auto& layers = map.getLayers();
    // we are loading a tile layer. not object or image
    assert(layers[layerIndex]->getType() == tmx::Layer::Type::Tile);

    const auto& layer = layers[layerIndex]->getLayerAs<tmx::TileLayer>();
    _className = layer.getClass();
    const auto  mapSize = map.getTileCount();
    const auto  mapTileSize = map.getTileSize();
    const auto& tileSets = map.getTilesets();

    const auto       tint = layer.getTintColour();
    const SDL_FColor vertColour = {tint.r / 255.f,
                                   tint.g / 255.f,
                                   tint.b / 255.f,
                                   tint.a / 255.f};

    // Map pixel space is Y-down, with (0,0) at the top-left of the map.
    // Convert to world units (meters, Y-up) so verts share a coord system with
    // MTransform. Map's bottom-left ends up at world (0,0); its top-left at
    // world (0, mapH_m).
    constexpr float invPTM = 1.f / megaman::GlobalData::PTM;
    const float mapHM = static_cast<float>(mapSize.y * mapTileSize.y) * invPTM;

    for (auto i = 0u; i < tileSets.size(); ++i)
    {
        const auto& ts = tileSets[i];
        const auto& tileIDs = layer.getTiles();

        if (textures.size() <= i || textures[i] == nullptr)
            return false;

        const auto texSize = textures[i]->getSize();
        const int  tileCountX = texSize.x / static_cast<int>(mapTileSize.x);

        // UV space: texture coordinates. (0,0) = top-left, (1,1) = bottom-right of texture

        // norms are size of 1 tile in UV space
        const float uNorm =
            static_cast<float>(mapTileSize.x) / static_cast<float>(texSize.x);
        const float vNorm =
            static_cast<float>(mapTileSize.y) / static_cast<float>(texSize.y);

        const float tileWM = static_cast<float>(mapTileSize.x) * invPTM;
        const float tileHM = static_cast<float>(mapTileSize.y) * invPTM;

        std::vector<SDL_Vertex> verts;
        for (auto y = 0u; y < mapSize.y; ++y)
        {
            for (auto x = 0u; x < mapSize.x; ++x)
            {
                const auto idx = y * mapSize.x + x;
                // skip tiles whose GID doesnt belong to current tileset
                if (idx >= tileIDs.size() ||
                    tileIDs[idx].ID < ts.getFirstGID() ||
                    tileIDs[idx].ID >= ts.getFirstGID() + ts.getTileCount())
                {
                    continue;
                }

                const auto localId = tileIDs[idx].ID - ts.getFirstGID();
                // calculate coordinates of tile in uv space
                float u =
                    static_cast<float>(localId %
                                       static_cast<std::uint32_t>(tileCountX)) *
                    static_cast<float>(mapTileSize.x) /
                    static_cast<float>(texSize.x);
                float v =
                    static_cast<float>(localId /
                                       static_cast<std::uint32_t>(tileCountX)) *
                    static_cast<float>(mapTileSize.y) /
                    static_cast<float>(texSize.y);

                // World-unit corner positions (Y-up).
                const float wxL = static_cast<float>(x) * tileWM;
                const float wxR = wxL + tileWM;
                const float wyT = mapHM - static_cast<float>(y) * tileHM;
                const float wyB = wyT - tileHM;

                _minX = std::min(_minX, wxL);
                _minY = std::min(_minY, wyB);

                // calculate tile vertex
                // Triangle1:
                // TL (UV top -> world top)
                verts.push_back({{wxL, wyT}, vertColour, {u, v}});
                // TR
                verts.push_back({{wxR, wyT}, vertColour, {u + uNorm, v}});
                // BL
                verts.push_back({{wxL, wyB}, vertColour, {u, v + vNorm}});
                // Triangle2:
                // BL
                verts.push_back({{wxL, wyB}, vertColour, {u, v + vNorm}});
                // TR
                verts.push_back({{wxR, wyT}, vertColour, {u + uNorm, v}});
                // BR
                verts.push_back(
                    {{wxR, wyB}, vertColour, {u + uNorm, v + vNorm}});
            }
        }

        if (!verts.empty())
        {
            _subsets.emplace_back();
            _subsets.back().texture = *textures[i];
            _subsets.back().vertexData.swap(verts);
        }
    }

    _maxX = _minX +
            static_cast<float>(map.getTileCount().x * mapTileSize.x) * invPTM;
    _maxY = _minY +
            static_cast<float>(map.getTileCount().y * mapTileSize.y) * invPTM;

    return !(_subsets.empty());
}

void MapTileLayer::draw(SDL_Renderer* renderer, const CameraData& cam) const
{
    assert(renderer);

    for (const auto& s : _subsets)
    {
        std::vector<SDL_Vertex> screenVerts{};
        screenVerts.reserve(s.vertexData.size());
        for (auto& v : s.vertexData)
        {
            auto vnew = v;
            vnew.position = megaman::worldToScreenPoint(v.position, cam);
            screenVerts.push_back(vnew);
        };

        // draw all verts using that texture
        SDL_RenderGeometry(renderer,
                           s.texture,
                           screenVerts.data(),
                           static_cast<int>(screenVerts.size()),
                           nullptr,
                           0);
    }
}
} // namespace megaman