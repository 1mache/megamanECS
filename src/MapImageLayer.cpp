#include "MapImageLayer.h"

#include "GlobalData.h"
#include "MTransform.h"

#include <tmxlite/ImageLayer.hpp>

#include <cassert>

namespace megaman
{
bool MapImageLayer::create(const tmx::Map& map,
                           std::uint32_t   layerIndex,
                           Texture*        texture)
{
    assert(texture);
    const auto& layers = map.getLayers();
    assert(layers[layerIndex]->getType() == tmx::Layer::Type::Image);

    const auto& layer = layers[layerIndex]->getLayerAs<tmx::ImageLayer>();

    constexpr float invPTM = 1.f / GlobalData::PTM;
    const auto      mapSize = map.getTileCount();
    const auto      mapTileSize = map.getTileSize();
    _mapWM = static_cast<float>(mapSize.x * mapTileSize.x) * invPTM;
    _mapHM = static_cast<float>(mapSize.y * mapTileSize.y) * invPTM;

    const auto imageSize = layer.getImageSize();
    const auto offset = layer.getOffset();

    // _transform stores bottom-left (x,y) and full size (w,h) — not MTransform center semantics.
    // Map pixel (0,0) is top-left; world (0,0) is map bottom-left.
    _transform.x = static_cast<float>(offset.x) * invPTM;
    _transform.y = _mapHM - (static_cast<float>(offset.y) +
                             static_cast<float>(imageSize.y)) *
                                invPTM;
    _transform.w = static_cast<float>(imageSize.x) * invPTM;
    _transform.h = static_cast<float>(imageSize.y) * invPTM;

    _repeatX = layer.hasRepeatX();
    _repeatY = layer.hasRepeatY();

    const auto p = layer.getParallaxFactor();
    _parallax = {p.x, p.y};

    const auto tint = layer.getTintColour();
    _tint = {tint.r / 255.f, tint.g / 255.f, tint.b / 255.f, tint.a / 255.f};

    _texture = *texture;
    return _texture != nullptr;
}

void MapImageLayer::draw(SDL_Renderer* renderer, const CameraData& cam) const
{
    assert(renderer);
    assert(_texture);

    // Parallax reference: camera position where viewport top-left aligns with map top-left
    // (matches Tiled's parallax model — offsets are relative to this origin, not world (0,0)).
    const float      ptmS = GlobalData::PTM * GlobalData::SCALE_FACTOR;
    const float      refCamX = GlobalData::getWinW() * 0.5f / ptmS;
    const float      refCamY = _mapHM - GlobalData::getWinH() * 0.5f / ptmS;
    const CameraData parallaxCam{refCamX + (cam.posX - refCamX) * _parallax.x,
                                 refCamY + (cam.posY - refCamY) * _parallax.y};

    const float ptmScaled = GlobalData::PTM * GlobalData::SCALE_FACTOR;
    const float wPx = _transform.w * ptmScaled;
    const float hPx = _transform.h * ptmScaled;

    const float endX = _repeatX ? _mapWM : _transform.x + _transform.w;
    const float endY = _repeatY ? _mapHM : _transform.y + _transform.h;

    SDL_SetTextureColorModFloat(_texture, _tint.r, _tint.g, _tint.b);
    SDL_SetTextureAlphaModFloat(_texture, _tint.a);

    for (float ty = _transform.y; ty < endY; ty += _transform.h)
    {
        for (float tx = _transform.x; tx < endX; tx += _transform.w)
        {
            // top-left world: (tx, ty + h) because Y-up, SDL draws from top-left
            const auto screenTL =
                worldToScreen({tx, ty + _transform.h}, parallaxCam);
            const SDL_FRect dst{.x = screenTL.x,
                                .y = screenTL.y,
                                .w = wPx,
                                .h = hPx};
            SDL_RenderTexture(renderer, _texture, nullptr, &dst);
        }
    }
}
} // namespace megaman
