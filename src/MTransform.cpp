#include "MTransform.h"
#include "GlobalData.h"
namespace megaman
{
SDL_FPoint worldToScreen(SDL_FPoint worldPos, const CameraData& cam)
{
    constexpr auto ptm = GlobalData::PTM;
    const float    s = GlobalData::SCALE_FACTOR;
    const float    cx = GlobalData::getWinW() * 0.5f;
    const float    cy = GlobalData::getWinH() * 0.5f;

    // Camera world coord -> screen center. Y-up world -> Y-down screen.
    return SDL_FPoint{cx + (worldPos.x - cam.posX) * ptm * s,
                      cy - (worldPos.y - cam.posY) * ptm * s};
}

SDL_FRect transformToFrect(const MTransform& t)
{
    constexpr auto ptm = GlobalData::PTM;
    const float    s = GlobalData::SCALE_FACTOR;
    const auto&    cam = GlobalData::getCamData();

    // Y-up world: top-left of AABB is (x - w, y + h).
    const auto topLeft = worldToScreen({t.x - t.w, t.y + t.h}, cam);

    return SDL_FRect{.x = topLeft.x,
                     .y = topLeft.y,
                     .w = t.w * 2 * ptm * s,
                     .h = t.h * 2 * ptm * s};
}
} // namespace megaman