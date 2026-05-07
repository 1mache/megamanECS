#pragma once
#include "CameraData.h"
#include <SDL3/SDL.h>
#include <cassert>
#include <utility>

namespace megaman
{
class GlobalData
{
public:
    static constexpr float PTM = 30.f;
    static constexpr float START_WIN_SCALE[] = {720, 540};
    static constexpr float START_CAM_POS[] = {0, 0};

public:
    static void setWindow(SDL_Window* window)
    {
        assert(window != nullptr && "setWindow: trying to pass nullptr");
        assert(_window == nullptr && "setWindow: trying to overwrite _window");
        _window = window;
    }

    static void setRenderer(SDL_Renderer* renderer)
    {
        assert(renderer != nullptr && "setRenderer: trying to pass nullptr");
        assert(_renderer == nullptr &&
               "setRenderer: trying to overwrite _renderer");
        _renderer = renderer;
    }

    static SDL_Window* getWindow()
    {
        assert(_window != nullptr && "getWindow: window was not created");
        return _window;
    }
    static SDL_Renderer* getRenderer()
    {
        assert(_renderer != nullptr && "getWindow: renderer was not created");
        return _renderer;
    }

    static CameraData& getCamData()
    {
        return _camData;
    }

private:
    GlobalData() = delete;
    GlobalData(const GlobalData&) = delete;
    void operator=(const GlobalData&) = delete;

    static inline SDL_Window*   _window{};
    static inline SDL_Renderer* _renderer{};
    static inline CameraData    _camData{};

    static inline float _winW{START_WIN_SCALE[0]}, _winH{START_WIN_SCALE[1]};
};
} // namespace megaman