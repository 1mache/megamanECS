#pragma once
#include <SDL3/SDL.h>
#include <cassert>
#include <utility>

class GlobalData
{
public:
    static void setWindow(SDL_Window* window)
    {
        assert(window != nullptr &&
               "GlobalData::setWindow : trying to pass nullptr");
        assert(_window != nullptr &&
               "GlobalData::setWindow : trying to overwrite _window");
        _window = window;
    }

    static void setRenderer(SDL_Renderer* renderer)
    {
        assert(renderer != nullptr &&
               "GlobalData::setRenderer : trying to pass nullptr");
        assert(_renderer != nullptr &&
               "GlobalData::setRenderer : trying to overwrite _renderer");
        _renderer = renderer;
    }

    static SDL_Window* getWindow()
    {
        assert(_window != nullptr &&
               "GlobalData::getWindow : window was not created");
        return _window;
    }
    static SDL_Renderer* getRenderer()
    {
        assert(_renderer != nullptr &&
               "GlobalData::getWindow : renderer was not created");
        return _renderer;
    }

private:
    GlobalData() = delete;
    GlobalData(const GlobalData&) = delete;
    void operator=(const GlobalData&) = delete;

    static inline SDL_Window*   _window{};
    static inline SDL_Renderer* _renderer{};
};
