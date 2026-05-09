#include "CameraData.h"
#include "GlobalData.h"
namespace megaman
{
SDL_FPoint camPosToScreenPos(const CameraData& cam)
{
    return SDL_FPoint{cam.posX * GlobalData::PTM,
                      GlobalData::getWinH() - cam.posY * GlobalData::PTM};
}
} // namespace megaman