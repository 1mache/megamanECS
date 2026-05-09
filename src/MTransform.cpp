#include "MTransform.h"
#include "GlobalData.h"
namespace megaman
{
SDL_FPoint worldToScreen(SDL_FPoint worldPos, const CameraData& cam)
{
    constexpr auto ptm = GlobalData::PTM;
    return SDL_FPoint{(worldPos.x - cam.posX) * ptm,
                      GlobalData::getWinH() - (worldPos.y - cam.posY) * ptm};
}

SDL_FRect transformToFrect(const MTransform& t)
{
    constexpr auto ptm = GlobalData::PTM;
    const auto&    cam = GlobalData::getCamData();

    // Y-up world: top-left of AABB is (x - w, y + h).
    const auto topLeft = worldToScreen({t.x - t.w, t.y + t.h}, cam);

    return SDL_FRect{.x = topLeft.x,
                     .y = topLeft.y,
                     .w = t.w * 2 * ptm,
                     .h = t.h * 2 * ptm};
}
} // namespace megaman