#include "GlobalData.h"
#include "MTransform.h"
#include "Scene.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <iostream>
#include <memory>
#include <vector>


int main(int, char**)
{
    using namespace megaman;
    using gd = GlobalData;

    float      scaleFactor = gd::SCALE_FACTOR;
    bool       inputLeft{}, inputRight{};
    MTransform cameraPos = {gd::START_CAM_X, gd::START_CAM_Y};

    float camVelX = 2.f;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    int         winW = static_cast<int>(gd::START_WIN_W);
    int         winH = static_cast<int>(gd::START_WIN_H);
    SDL_Window* window =
        SDL_CreateWindow("tmxlite SDL3 Example", winW, winH, 0);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderVSync(renderer, 1);
    gd::setWindow(window);
    gd::setRenderer(renderer);

    Scene scene{"res/map/map.tmx"};
    scene.load(renderer);

    SDL_SetRenderDrawColor(renderer, 50, 50, 100, 255);

    bool running = true;
    while (running)
    {
        SDL_Event evt;
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_EVENT_QUIT)
                running = false;
            else if (evt.type == SDL_EVENT_KEY_DOWN &&
                     evt.key.key == SDLK_ESCAPE)
                running = false;
            else if (evt.type == SDL_EVENT_KEY_DOWN && evt.key.key == SDLK_A)
                inputLeft = true;
            else if (evt.type == SDL_EVENT_KEY_UP && evt.key.key == SDLK_A)
                inputLeft = false;
            else if (evt.type == SDL_EVENT_KEY_DOWN && evt.key.key == SDLK_D)
                inputRight = true;
            else if (evt.type == SDL_EVENT_KEY_UP && evt.key.key == SDLK_D)
                inputRight = false;
        }

        const auto& camData = gd::getCamData();
        if (inputRight)
        {
            gd::updateCamPosition(camData.posX + (camVelX * gd::FRAME_DELTA_MS),
                                  camData.posY);
        }
        if (inputLeft)
        {
            gd::updateCamPosition(camData.posX - (camVelX * gd::FRAME_DELTA_MS),
                                  camData.posY);
        }

        SDL_RenderClear(renderer);
        scene.draw(renderer, gd::getCamData());
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
