#include "Scene.h"

#include <tmxlite/Map.hpp>

#include <cassert>
#include <iostream>

namespace megaman
{
void loadScene(Scene& scene, SDL_Renderer* renderer)
{
    tmx::Map map;
    if (map.load(scene.filePath))
    {
        // load all tileset textures
        for (const auto& ts : map.getTilesets())
        {
            scene.textures.emplace_back(std::make_unique<Texture>());
            if (!scene.textures.back()->loadFromFile(ts.getImagePath(), renderer))
                std::cerr << "Failed to load tileset: " << ts.getImagePath()
                          << "\n";
        }

        // create all map layers
        const auto& mapLayers = map.getLayers();
        for (auto i = 0u; i < mapLayers.size(); ++i)
        {
            if (mapLayers[i]->getType() == tmx::Layer::Type::Tile)
            {
                scene.tileLayers.emplace_back(std::make_unique<MapTileLayer>());
                scene.tileLayers.back()->create(map, i, scene.textures);
            }
        }
    }
    else
    {
        std::cerr << "Failed to load map\n";
    }
}

void drawScene(const Scene& scene, SDL_Renderer* renderer, SDL_FPoint camOffset)
{
    assert(isSceneValid(scene));
    for (const auto& l : scene.tileLayers)
        l->draw(renderer, camOffset);
}
} // namespace megaman
