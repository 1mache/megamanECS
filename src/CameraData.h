#pragma once
#include <SDL3/SDL.h>

namespace megaman
{
struct CameraData
{
    float posX{}, posY{};
};

SDL_FPoint camPosToScreenPos(const CameraData& cam);
} // namespace megaman
