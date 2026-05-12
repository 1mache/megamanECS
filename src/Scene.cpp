#include "Scene.h"

#include "GlobalData.h"
#include "MTransform.h"

#include <tmxlite/ImageLayer.hpp>
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
        const auto& tileSets = map.getTilesets();
        assert(tileSets.size() == 1 &&
               "Multiple or no tilesets provided. Not supported");

        _textures.emplace_back(std::make_unique<Texture>());
        if (!_textures.back()->loadFromFile(tileSets[0].getImagePath(), renderer))
            std::cerr << "Failed to load tileset: " << tileSets[0].getImagePath()
                      << "\n";

        const auto& mapLayers = map.getLayers();
        for (auto i = 0u; i < mapLayers.size(); ++i)
        {
            if (mapLayers[i]->getType() == tmx::Layer::Type::Tile)
            {
                processTileLayer(mapLayers[i], i, map);
            }
            else if (mapLayers[i]->getType() == tmx::Layer::Type::Object)
            {
                if (mapLayers[i]->getClass() == SOLID_CLASS)
                    processCollisionLayer(mapLayers[i], i, map);
            }
            else if (mapLayers[i]->getType() == tmx::Layer::Type::Image)
            {
                processImageLayer(renderer, mapLayers[i], i, map);
            }
        }
        _loaded = true;
    }
    else
    {
        std::cerr << "Failed to load map\n";
    }
}

void Scene::attachPhysics(b2WorldId worldId)
{
    for (const auto& l : _collisionLayers)
    {
        l->createBodies(worldId);
    }
}

void Scene::draw(SDL_Renderer* renderer, const CameraData& cam) const
{
    assert(isValid());
    for (const auto& l : _imageLayers)
        l->draw(renderer, cam);
    for (const auto& l : _tileLayers)
        l->draw(renderer, cam);
}

WorldBoundsM Scene::getBoundsM() const
{
    if (_tileLayers.empty())
        return {};

    auto bounds = _tileLayers.front()->getBoundsM();
    // find min/max across all layers
    for (std::size_t i = 1; i < _tileLayers.size(); ++i)
    {
        const auto bi = _tileLayers[i]->getBoundsM();
        bounds.minX = std::min(bounds.minX, bi.minX);
        bounds.maxX = std::max(bounds.maxX, bi.maxX);
        bounds.minY = std::min(bounds.minY, bi.minY);
        bounds.maxY = std::max(bounds.maxY, bi.maxY);
    }
    return bounds;
}

void Scene::clampCameraToBounds(CameraData& cam) const
{
    if (!isValid())
        return;

    const auto  b = getBoundsM();
    const float halfW = screenToWorldSize(GlobalData::getWinW() * 0.5f);
    const float halfH = screenToWorldSize(GlobalData::getWinH() * 0.5f);

    // calculate min and max cam center positions
    const float minCx =
        b.minX + halfW; // leftmost center where left edge touches b.minX
    const float maxCx =
        b.maxX - halfW; // rightmost center where right edge touches b.maxX
    const float minCy = b.minY + halfH; // similarly for Y
    const float maxCy = b.maxY - halfH;

    // View bigger than map on an axis -> lock to map center on that axis.
    // otherwise clamp
    // maxCx - minCx = (b.maxX - halfW) - (b.minX + halfW)
    //               = mapWidth - viewWidth
    cam.posX = (minCx > maxCx) ? (b.minX + b.maxX) * 0.5f
                               : std::clamp(cam.posX, minCx, maxCx);
    cam.posY = (minCy > maxCy) ? (b.minY + b.maxY) * 0.5f
                               : std::clamp(cam.posY, minCy, maxCy);
}

void Scene::processTileLayer(const tmx::Layer::Ptr& layer,
                             unsigned int           idx,
                             const tmx::Map&        map)
{
    _tileLayers.emplace_back(std::make_unique<MapTileLayer>());
    bool created = _tileLayers.back()->create(map, idx, _textures[0]);
    if (!created)
        std::cerr << "Failed to create tile layer. Id: " << idx << "\n";

    // TODO: physics here
    if (_tileLayers.back()->getClassName() == SOLID_CLASS)
        std::cout << "Loaded tile layer marked solid. Id: " << idx << "\n";
}

void Scene::processImageLayer(SDL_Renderer*          renderer,
                              const tmx::Layer::Ptr& layer,
                              unsigned int           idx,
                              const tmx::Map&        map)
{
    const auto& imgLayer = layer->getLayerAs<tmx::ImageLayer>();
    _textures.emplace_back(std::make_unique<Texture>());
    if (!_textures.back()->loadFromFile(imgLayer.getImagePath(), renderer))
        std::cerr << "Failed to load image layer: " << imgLayer.getImagePath()
                  << "\n";

    _imageLayers.emplace_back(std::make_unique<MapImageLayer>());
    bool created = _imageLayers.back()->create(map, idx, _textures.back().get());
    if (!created)
        std::cerr << "Failed to create image layer. Id: " << idx << "\n";
}
void Scene::processCollisionLayer(const tmx::Layer::Ptr& layer,
                                  unsigned int           idx,
                                  const tmx::Map&        map)
{
    const auto& objLayer = layer->getLayerAs<tmx::ObjectGroup>();
    _collisionLayers.emplace_back(std::make_unique<MapCollisionLayer>());
    bool created = _collisionLayers.back()->create(map, idx);
    if (!created)
        std::cerr << "Failed to create collision layer. Id: " << idx << "\n";
}
} // namespace megaman
