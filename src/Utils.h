/**
 * @file Utils.h
 * @brief Fatal-error reporting and debug/release assertion helper.
 */
#pragma once
#include <SDL3/SDL.h>
#include <cassert>
#include <cstdlib>

namespace megaman
{
/** @brief Logs @p message, shows a message box, and terminates immediately. */
[[noreturn]] inline void fatalError(const char* message)
{
    SDL_Log("Fatal error: %s", message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message, nullptr);
    SDL_Quit();
    std::exit(1);
}

/**
 * @brief Debug/release assertion. Calls assert() in debug builds; calls fatalError() in release.
 * @param condition  Expression that must be true.
 * @param message    Shown if the condition fails.
 */
inline void massert(bool condition, const char* message = "Assertion failed")
{
#ifndef NDEBUG
    assert(condition && message);
#else
    if (!condition)
        fatalError(message);
#endif
}
} // namespace megaman
