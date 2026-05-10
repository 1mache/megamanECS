#include "Scene.h"

#include "GlobalData.h"

#include <tmxlite/Map.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>

namespace megaman
{
void Scene::load(SDL_Renderer* renderer)
{
    tmx::Map map;
    if (map.load(_filePath))
    {
        for (const auto& ts : map.getTilesets())
        {
            _textures.emplace_back(std::make_unique<Texture>());
            if (!_textures.back()->loadFromFile(ts.getImagePath(), renderer))
                std::cerr << "Failed to load tileset: " << ts.getImagePath()
                          << "\n";
        }

        const auto& mapLayers = map.getLayers();
        for (auto i = 0u; i < mapLayers.size(); ++i)
        {
            if (mapLayers[i]->getType() == tmx::Layer::Type::Tile)
            {
                _tileLayers.emplace_back(std::make_unique<MapTileLayer>());
                _tileLayers.back()->create(map, i, _textures);
            }
        }
        _loaded = true;
    }
    else
    {
        std::cerr << "Failed to load map\n";
    }
}

void Scene::draw(SDL_Renderer* renderer, const CameraData& cam) const
{
    assert(isValid());
    for (const auto& l : _tileLayers)
        l->draw(renderer, cam);
}

WorldBoundsM Scene::getBoundsM() const
{
    if (_tileLayers.empty())
        return {};

    auto b = _tileLayers.front()->getBoundsM();
    for (std::size_t i = 1; i < _tileLayers.size(); ++i)
    {
        const auto bi = _tileLayers[i]->getBoundsM();
        b.minX = std::min(b.minX, bi.minX);
        b.maxX = std::max(b.maxX, bi.maxX);
        b.minY = std::min(b.minY, bi.minY);
        b.maxY = std::max(b.maxY, bi.maxY);
    }
    return b;
}

void Scene::clampCameraToBounds(CameraData& cam) const
{
    if (!isValid())
        return;

    const auto  b = getBoundsM();
    const float invPTMS =
        1.f / (GlobalData::PTM * GlobalData::SCALE_FACTOR);
    const float halfW = GlobalData::getWinW() * 0.5f * invPTMS;
    const float halfH = GlobalData::getWinH() * 0.5f * invPTMS;

    const float minCx = b.minX + halfW;
    const float maxCx = b.maxX - halfW;
    const float minCy = b.minY + halfH;
    const float maxCy = b.maxY - halfH;

    // View bigger than map on an axis -> lock to map center on that axis.
    cam.posX = (minCx > maxCx) ? (b.minX + b.maxX) * 0.5f
                               : std::clamp(cam.posX, minCx, maxCx);
    cam.posY = (minCy > maxCy) ? (b.minY + b.maxY) * 0.5f
                               : std::clamp(cam.posY, minCy, maxCy);
}
} // namespace megaman
