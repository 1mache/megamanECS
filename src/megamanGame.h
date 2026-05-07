#pragma once

#include "megaman.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace megaman
{
class MegamanGame
{
public:
    MegamanGame();
    ~MegamanGame();

    bool valid() const
    {
        return _win != nullptr && _ren != nullptr;
    }
    void run();

private:
    static constexpr int    HP = 3;
    static constexpr int    WIN_W = 720;
    static constexpr int    WIN_H = 540;
    static constexpr int    FPS = 60;
    static constexpr Uint64 GAME_FRAME = 1000 / FPS;

    void inputSystem();
    void moveSystem();
    void boxSystem();
    void drawSystem();
    void animationSystem();

    SDL_Window*   _win = nullptr;
    SDL_Renderer* _ren = nullptr;
    SDL_Texture*  _tex = nullptr;

    b2WorldId _box = b2_nullWorldId;
};
} // namespace megaman