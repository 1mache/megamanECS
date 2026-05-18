/**
 * @file GlobalData.h
 * @brief Singleton-style static store for engine-wide resources and constants.
 *
 * Unit system:
 *  - World coordinates are in **meters** (Y-up).
 *  - PTM (pixels-to-meters) = 16: one meter equals 16 texels at native scale.
 *  - SCALE_FACTOR = 2.6: the renderer scales texels up by this factor.
 *  - Combined: 1 m = PTM × SCALE_FACTOR ≈ 41.6 screen pixels.
 *  - GRAVITY is in m/s².
 */
#pragma once
#include "CameraData.h"
#include "Utils.h"
#include "WorldBoundsM.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace megaman
{
/**
 * @brief Engine-global resources: window, renderer, textures, Box2D world, camera.
 *
 * All members are static; the class cannot be instantiated. Setters assert-once
 * (debug) or fatal-error (release) to prevent accidental overwrites.
 */
class GlobalData
{
public:
    static constexpr int   FPS            = 60;
    static constexpr float FRAME_DELTA_MS = FPS ? (1000.f / FPS) : 0.f;
    static constexpr float PTM            = 16.f;
    static constexpr float START_WIN_W    = 720;
    static constexpr float START_WIN_H    = 540;
    static constexpr float START_CAM_X    = (START_WIN_W / (2 * PTM)) - 8.f;
    static constexpr float START_CAM_Y    = (START_WIN_H / (2 * PTM)) - 5.5f;
    static constexpr float SCALE_FACTOR   = 2.6f;
    static constexpr float GRAVITY        = 25.f;

    static void setWindow(SDL_Window* window)
    {
        massert(window != nullptr, "setWindow: trying to pass nullptr");
        massert(_window == nullptr, "setWindow: trying to overwrite _window");
        _window = window;
    }

    static void setRenderer(SDL_Renderer* renderer)
    {
        massert(renderer != nullptr, "setRenderer: trying to pass nullptr");
        massert(_renderer == nullptr, "setRenderer: trying to overwrite _renderer");
        _renderer = renderer;
    }

    static void setShotTexture(SDL_Texture* tex)
    {
        massert(tex != nullptr, "setShotTexture: trying to pass nullptr");
        _shotTex = tex;
    }

    static SDL_Texture* getShotTexture()
    {
        return _shotTex;
    }

    static void setExplosionTexture(SDL_Texture* tex)
    {
        massert(tex != nullptr, "setExplosionTexture: trying to pass nullptr");
        _explosionTex = tex;
    }

    static SDL_Texture* getExplosionTexture()
    {
        return _explosionTex;
    }

    static void setBoxWorld(b2WorldId worldId)
    {
        _boxWorld = worldId;
    }

    static b2WorldId getBoxWorld()
    {
        return _boxWorld;
    }

    static SDL_Window* getWindow()
    {
        massert(_window != nullptr, "getWindow: window was not created");
        return _window;
    }

    static SDL_Renderer* getRenderer()
    {
        massert(_renderer != nullptr, "getRenderer: renderer was not created");
        return _renderer;
    }

    static float getWinW()
    {
        return START_WIN_W;
    }

    static float getWinH()
    {
        return START_WIN_H;
    }

    static float getScaleFactor()
    {
        return SCALE_FACTOR;
    }

    static const CameraData& getCamData()
    {
        return _camData;
    }

    static void updateCamPosition(float x, float y)
    {
        _camData.posX = x;
        _camData.posY = y;
    }

    /**
     * @brief Computes the camera's visible AABB in world units.
     *
     * Converts the half-viewport extents from screen pixels to meters using
     * PTM × SCALE_FACTOR, then offsets from the current camera center.
     * Used by projectileCullSystem and any system that needs world-space culling.
     *
     * @return WorldBoundsM covering the currently visible area.
     */
    static WorldBoundsM getCamBoundsM()
    {
        constexpr float ppm   = PTM * SCALE_FACTOR;
        const float     halfW = START_WIN_W * 0.5f / ppm;
        const float     halfH = START_WIN_H * 0.5f / ppm;
        return {_camData.posX - halfW,
                _camData.posX + halfW,
                _camData.posY - halfH,
                _camData.posY + halfH};
    }

private:
    GlobalData()                      = delete;
    GlobalData(const GlobalData&)     = delete;
    void operator=(const GlobalData&) = delete;

    static inline SDL_Window*   _window{};
    static inline SDL_Renderer* _renderer{};
    static inline SDL_Texture*  _shotTex{};
    static inline SDL_Texture*  _explosionTex{};
    static inline b2WorldId     _boxWorld{};
    static inline CameraData    _camData{START_CAM_X, START_CAM_Y};
};
} // namespace megaman
