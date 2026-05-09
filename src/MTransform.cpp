#include "MTransform.h"
#include "GlobalData.h"
namespace megaman
{
SDL_FRect transformToFrect(const MTransform& t)
{
    constexpr auto ptm = GlobalData::PTM;
    const auto& cam = GlobalData::getCamData();

    auto transx = t.x - cam.posX;
    auto transy = t.y - cam.posY;

    return SDL_FRect{.x = (transx - t.w) * ptm,
                     .y = GlobalData::START_WIN_H - ((transy + t.h) * ptm),
                     .w = t.w * 2 * ptm,
                     .h = t.h * 2 * ptm};
}
} // namespace megaman