#include "MapLayer.h"
#include "Texture.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <tmxlite/Map.hpp>

#include <iostream>
#include <memory>
#include <vector>

constexpr float SCALE_FACTOR = 2;

int main(int, char**)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("tmxlite SDL3 Example", 800, 600, 0);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderScale(renderer, SCALE_FACTOR, SCALE_FACTOR);
    SDL_SetRenderVSync(renderer, 1);

    std::vector<std::unique_ptr<Texture>>  textures;
    std::vector<std::unique_ptr<MapLayer>> renderLayers;

    tmx::Map map;
    if (map.load("res/map/map.tmx"))
    {
        // load all tileset textures
        for (const auto& ts : map.getTilesets())
        {
            textures.emplace_back(std::make_unique<Texture>());
            if (!textures.back()->loadFromFile(ts.getImagePath(), renderer))
                std::cerr << "Failed to load tileset: " << ts.getImagePath() << "\n";
        }

        // create all map layers
        const auto& mapLayers = map.getLayers();
        for (auto i = 0u; i < mapLayers.size(); ++i)
        {
            if (mapLayers[i]->getType() == tmx::Layer::Type::Tile)
            {
                renderLayers.emplace_back(std::make_unique<MapLayer>());
                renderLayers.back()->create(map, i, textures);
            }
        }
    }
    else
    {
        std::cerr << "Failed to load map\n";
    }

    SDL_SetRenderDrawColor(renderer, 50, 50, 100, 255);

    bool running = true;
    while (running)
    {
        SDL_Event evt;
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_EVENT_QUIT)
                running = false;
            else if (evt.type == SDL_EVENT_KEY_DOWN && evt.key.key == SDLK_ESCAPE)
                running = false;
        }

        SDL_RenderClear(renderer);
        for (const auto& l : renderLayers)
            l->draw(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
