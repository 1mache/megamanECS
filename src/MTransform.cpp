#include "MTransform.h"
namespace megaman
{
SDL_FRect transformToFrect(const MTransform& t)
{
    auto ptm = GlobalData::PTM;
    auto camData = GlobalData::getCamData();

    // position offseted by camera position
    auto transx = t.x - camData.posX;
    auto transy = t.y - camData.posY;

    // invert y and convert to pixels
    return SDL_FRect{.x = (transx - t.w) * ptm,
                     .y = camData.vpH - ((transy + t.h) * ptm),
                     .w = t.w * 2 * ptm,
                     .h = t.h * 2 * ptm};
}
} // namespace megaman