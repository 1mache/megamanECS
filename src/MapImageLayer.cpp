#include "MapImageLayer.h"

#include "GlobalData.h"
#include "MTransform.h"

#include <tmxlite/ImageLayer.hpp>

#include "Utils.h"

namespace megaman
{
bool MapImageLayer::create(const tmx::Map& map,
                           std::uint32_t   layerIndex,
                           Texture*        texture)
{
    massert(texture, "Passed nullptr texture to MapImageLayer::create");
    const auto& layers = map.getLayers();
    massert(layerIndex < layers.size(), "Layer index out of bounds");
    massert(layers[layerIndex]->getType() == tmx::Layer::Type::Image,
            "Layer at index is not an image layer");

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
    _transform.y =
        _mapHM -
        (static_cast<float>(offset.y) + static_cast<float>(imageSize.y)) * invPTM;
    _transform.w = static_cast<float>(imageSize.x) * invPTM;
    _transform.h = static_cast<float>(imageSize.y) * invPTM;

    _repeatX = layer.hasRepeatX();
    _repeatY = layer.hasRepeatY();

    const auto p = layer.getParallaxFactor();
    _parallax = {p.x, p.y};
    _parallaxRef = {map.getParallaxOrigin().x, map.getParallaxOrigin().y};
    // translate to world space
    _parallaxRef.x *= invPTM;
    _parallaxRef.y = _mapHM - (_parallaxRef.y * invPTM);

    const auto tint = layer.getTintColour();
    _tint = {tint.r / 255.f, tint.g / 255.f, tint.b / 255.f, tint.a / 255.f};

    _texture = *texture;
    return isValid();
}

void MapImageLayer::draw(SDL_Renderer* renderer, const CameraData& cam) const
{
    massert(renderer);
    massert(isValid(),
            "Must call create() successfully before drawing MapImageLayer");

    // Parallax reference: camera position where viewport top-left aligns with map top-left
    // (matches Tiled's parallax model — offsets are relative to this origin, not world (0,0)).
    const CameraData parallaxCam{
        _parallaxRef.x + (cam.posX - _parallaxRef.x) * _parallax.x,
        _parallaxRef.y + (cam.posY - _parallaxRef.y) * _parallax.y};

    const float wPx = worldToScreenSize(_transform.w);
    const float hPx = worldToScreenSize(_transform.h);

    // in world size (half viewport extents in world units)
    const float halfW = screenToWorldSize(GlobalData::getWinW() * 0.5f);
    const float halfH = screenToWorldSize(GlobalData::getWinH() * 0.5f);

    // Calculate image bounds
    float startX = _transform.x;
    float endX = _transform.x + _transform.w;
    float startY = _transform.y;
    float endY = _transform.y + _transform.h;

    if (_repeatX)
    {
        // Find first tile position to left of parallax viewport
        startX = floor((parallaxCam.posX - halfW - _transform.x) / _transform.w) *
                     _transform.w +
                 _transform.x;
        endX = parallaxCam.posX + halfW + _transform.w;
    }

    if (_repeatY)
    {
        startY = floor((parallaxCam.posY - halfH - _transform.y) / _transform.h) *
                     _transform.h +
                 _transform.y;
        endY = parallaxCam.posY + halfH + _transform.h;
    }

    SDL_SetTextureColorModFloat(_texture, _tint.r, _tint.g, _tint.b);
    SDL_SetTextureAlphaModFloat(_texture, _tint.a);

    for (float ty = startY; ty < endY; ty += _transform.h)
    {
        for (float tx = startX; tx < endX; tx += _transform.w)
        {
            // top-left world: (tx, ty + h) because Y-up, SDL draws from top-left
            auto screenTL = worldToScreenPoint({tx, ty + _transform.h}, parallaxCam);
            SDL_FRect dst{.x = screenTL.x, .y = screenTL.y, .w = wPx, .h = hPx};
            SDL_RenderTexture(renderer, _texture, nullptr, &dst);
        }
    }
}
} // namespace megaman
