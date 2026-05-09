#pragma once
#include "MapTileLayer.h"
#include "Texture.h"
#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <vector>

namespace megaman
{
struct Scene
{
    // Preferred storage: Sparse, probably only one entity of this type with
    // small id value so one short array is good.

    std::string                                filePath{};
    bool                                       loaded{};
    std::vector<std::unique_ptr<Texture>>      textures;
    std::vector<std::unique_ptr<MapTileLayer>> tileLayers;
};

inline bool isSceneValid(const Scene& s)
{
    return s.loaded && !s.tileLayers.empty();
}

void loadScene(Scene& scene, SDL_Renderer* renderer);
void drawScene(const Scene&  scene,
               SDL_Renderer* renderer,
               SDL_FPoint    camOffset);

} // namespace megaman
