#include "Scene.h"

namespace megaman
{
void Scene::load(SDL_Renderer* renderer)
{
    tmx::Map map;
    if (map.load(_filePath))
    {
        // load all tileset textures
        for (const auto& ts : map.getTilesets())
        {
            _textures.emplace_back(std::make_unique<Texture>());
            if (!_textures.back()->loadFromFile(ts.getImagePath(), renderer))
                std::cerr << "Failed to load tileset: " << ts.getImagePath()
                          << "\n";
        }

        // create all map layers
        const auto& mapLayers = map.getLayers();
        for (auto i = 0u; i < mapLayers.size(); ++i)
        {
            if (mapLayers[i]->getType() == tmx::Layer::Type::Tile)
            {
                _tileLayers.emplace_back(std::make_unique<MapTileLayer>());
                _tileLayers.back()->create(map, i, _textures);
            }
        }
    }
    else
    {
        std::cerr << "Failed to load map\n";
    }
}

void Scene::draw(SDL_Renderer* renderer, SDL_FPoint camOffset)
{
    assert(isValid());
    for (const auto& l : _tileLayers)
        l->draw(renderer, camOffset);
}
} // namespace megaman