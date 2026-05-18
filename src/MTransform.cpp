/**
 * @file MTransform.cpp
 * @brief worldToScreenPoint and transformToFrect implementations.
 */
#include "MTransform.h"
#include "GlobalData.h"
namespace megaman
{
SDL_FPoint worldToScreenPoint(SDL_FPoint worldPos, const CameraData& cam)
{
    constexpr auto ptm = GlobalData::PTM;
    const float    s = GlobalData::SCALE_FACTOR;
    const float    centx = GlobalData::getWinW() * 0.5f;
    const float    centy = GlobalData::getWinH() * 0.5f;

    // Camera world coord -> screen center. Y-up world -> Y-down screen.
    return SDL_FPoint{centx + (worldPos.x - cam.posX) * ptm * s,
                      centy - (worldPos.y - cam.posY) * ptm * s};
}

SDL_FRect transformToFrect(const MTransform& t)
{
    constexpr auto ptm = GlobalData::PTM;
    const float    s = GlobalData::SCALE_FACTOR;
    const auto&    cam = GlobalData::getCamData();

    // Y-up world: top-left of AABB is (x - w, y + h).
    const auto topLeft = worldToScreenPoint({t.x - t.w, t.y + t.h}, cam);

    return SDL_FRect{.x = topLeft.x,
                     .y = topLeft.y,
                     .w = worldToScreenSize(t.w * 2),
                     .h = worldToScreenSize(t.h * 2)};
}
} // namespace megaman