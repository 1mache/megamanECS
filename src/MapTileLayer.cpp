#include "MapTileLayer.h"

#include "GlobalData.h"
#include "MTransform.h"

#include <tmxlite/TileLayer.hpp>

#include <cassert>
namespace megaman
{
bool MapTileLayer::create(const tmx::Map&                 map,
                          std::uint32_t                   layerIndex,
                          const std::unique_ptr<Texture>& texture)
{
    _texture = *texture;
    const auto& layers = map.getLayers();
    // we are loading a tile layer. not object or image
    assert(layers[layerIndex]->getType() == tmx::Layer::Type::Tile &&
           "Layer index does not point to a tile layer");

    const auto& layer = layers[layerIndex]->getLayerAs<tmx::TileLayer>();
    _className = layer.getClass();

    const auto mapSize = map.getTileCount();
    const auto mapTileSize = map.getTileSize();

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
    const float     mapHM = static_cast<float>(mapSize.y * mapTileSize.y) * invPTM;

    const auto& ts = map.getTilesets()[0]; // the first and only tileset
    const auto& tileIDs = layer.getTiles();

    const auto texSize = texture->getSize();
    const int  tileCountX = texSize.x / static_cast<int>(mapTileSize.x);

    // UV space: texture coordinates. (0,0) = top-left, (1,1) = bottom-right of texture

    // norms are size of 1 tile in UV space
    const float uNorm =
        static_cast<float>(mapTileSize.x) / static_cast<float>(texSize.x);
    const float vNorm =
        static_cast<float>(mapTileSize.y) / static_cast<float>(texSize.y);

    const float tileWM = static_cast<float>(mapTileSize.x) * invPTM;
    const float tileHM = static_cast<float>(mapTileSize.y) * invPTM;

    _vertexData.reserve(mapSize.x * mapSize.y * 4); // 4 verts per tile (square)
    _indices.reserve(mapSize.x * mapSize.y * 6); // 6 indices per tile (2 triangles)
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
            float u = static_cast<float>(localId %
                                         static_cast<std::uint32_t>(tileCountX)) *
                      static_cast<float>(mapTileSize.x) /
                      static_cast<float>(texSize.x);
            float v = static_cast<float>(localId /
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

            int vertIdx = static_cast<int>(_vertexData.size());
            // calculate tile vertex
            // TL (UV top -> world top)
            _vertexData.push_back({{wxL, wyT}, vertColour, {u, v}});
            // TR
            _vertexData.push_back({{wxR, wyT}, vertColour, {u + uNorm, v}});
            // BL
            _vertexData.push_back({{wxL, wyB}, vertColour, {u, v + vNorm}});
            // BR
            _vertexData.push_back({{wxR, wyB}, vertColour, {u + uNorm, v + vNorm}});

            // Triangle1:
            _indices.push_back(vertIdx);     // TL
            _indices.push_back(vertIdx + 1); // TR
            _indices.push_back(vertIdx + 2); // BL
            // Triangle2:
            _indices.push_back(vertIdx + 2); // BL
            _indices.push_back(vertIdx + 1); // TR
            _indices.push_back(vertIdx + 3); // BR
        }
    }

    _maxX =
        _minX + static_cast<float>(map.getTileCount().x * mapTileSize.x) * invPTM;
    _maxY =
        _minY + static_cast<float>(map.getTileCount().y * mapTileSize.y) * invPTM;

    return !(_vertexData.empty());
}

void MapTileLayer::draw(SDL_Renderer* renderer, const CameraData& cam) const
{
    assert(renderer);

    std::vector<SDL_Vertex> screenVerts{};
    screenVerts.reserve(_vertexData.size());
    for (auto& v : _vertexData)
    {
        auto vnew = v;
        vnew.position = megaman::worldToScreenPoint(v.position, cam);
        screenVerts.push_back(vnew);
    };

    // draw all verts using that texture
    SDL_RenderGeometry(renderer,
                       _texture,
                       screenVerts.data(),
                       static_cast<int>(screenVerts.size()),
                       _indices.data(),
                       static_cast<int>(_indices.size()));
}
} // namespace megaman