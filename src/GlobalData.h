#pragma once
#include "CameraData.h"
#include <SDL3/SDL.h>
#include <cassert>

namespace megaman
{
    class GlobalData
    {
    public:
        static constexpr int   FPS = 60;
        static constexpr float FRAME_DELTA_MS = FPS ? (1000.f / FPS) : 0.f;
        static constexpr float PTM = 16.f;
        static constexpr float START_WIN_W = 720;
        static constexpr float START_WIN_H = 540;
        static constexpr float START_CAM_X = (START_WIN_W / (2 * PTM)) - 8.f;
        static constexpr float START_CAM_Y = (START_WIN_H / (2 * PTM)) - 5.5f;
        static constexpr float SCALE_FACTOR = 2.f;
        static constexpr float GRAVITY = 20.f;

        static void setWindow(SDL_Window *window)
        {
            assert(window != nullptr && "setWindow: trying to pass nullptr");
            assert(_window == nullptr && "setWindow: trying to overwrite _window");
            _window = window;
        }

        static void setRenderer(SDL_Renderer *renderer)
        {
            assert(renderer != nullptr && "setRenderer: trying to pass nullptr");
            assert(_renderer == nullptr &&
                   "setRenderer: trying to overwrite _renderer");
            _renderer = renderer;
        }

        static void setShotTexture(SDL_Texture *tex)
        {
            assert(tex != nullptr && "setShotTexture: trying to pass nullptr");
            _shotTex = tex;
        }

        static SDL_Texture *getShotTexture()
        {
            return _shotTex;
        }

        static SDL_Window *getWindow()
        {
            assert(_window != nullptr && "getWindow: window was not created");
            return _window;
        }

        static SDL_Renderer *getRenderer()
        {
            assert(_renderer != nullptr && "getRenderer: renderer was not created");
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

        static const CameraData &getCamData()
        {
            return _camData;
        }

        static void updateCamPosition(float x, float y)
        {
            _camData.posX = x;
            _camData.posY = y;
        }

    private:
        GlobalData() = delete;
        GlobalData(const GlobalData &) = delete;
        void operator=(const GlobalData &) = delete;

        static inline SDL_Window   *_window{};
        static inline SDL_Renderer *_renderer{};
        static inline SDL_Texture  *_shotTex{};
        static inline CameraData    _camData{START_CAM_X, START_CAM_Y};
    };
} // namespace megaman
