#pragma once
#include <SDL3/SDL.h>
#include <cassert>
#include <cstdlib>

namespace megaman
{
[[noreturn]] inline void fatalError(const char* message)
{
    SDL_Log("Fatal error: %s", message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message, nullptr);
    SDL_Quit();
    std::exit(1);
}

inline void massert(bool condition, const char* message)
{
#ifndef NDEBUG
    assert(condition && message);
#else
    if (!condition)
        fatalError(message);
#endif
}
} // namespace megaman
