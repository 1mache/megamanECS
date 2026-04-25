#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <iostream>

constexpr int             WIN_WIDTH = 720;
constexpr int             WIN_HEIGHT = 405;
constexpr SDL_WindowFlags WIN_FLAGS = 0;

bool createWindowAndRenderer(const char*    title,
                             SDL_Window*&   window,
                             SDL_Renderer*& renderer)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Init error : " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(title, WIN_WIDTH, WIN_HEIGHT, WIN_FLAGS);

    if (!window)
    {
        std::cerr << "Window creation error : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    SDL_SetWindowPosition(window,
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        std::cerr << "Renderer creation error : " << SDL_GetError()
                  << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    return true;
}

void destroyResourcesAndQuit(SDL_Window* window, SDL_Renderer* renderer)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main()
{
    SDL_Window*   window{};
    SDL_Renderer* renderer{};

    if (!createWindowAndRenderer("Megaman Mockup", window, renderer))
        return EXIT_FAILURE;

    bool isRunning = true;


    SDL_Surface* surface = IMG_Load("res/area.png");
    if (!surface)
    {
        std::cerr << "Image loading error : " << SDL_GetError() << std::endl;
        destroyResourcesAndQuit(window, renderer);
        return EXIT_FAILURE;
    }

    // 1.load scene picture
    // 2.init player
    // 3.init enemy
    // 4.init health indication
    // 5.init ground collider


    SDL_Event* event{};

    while (isRunning)
    {
        // 1.  move player (+ animate)
        // check player ground position
        // 2.  move enemy (hover)
        // 3.  if player at (x1,y1) position => jumps + switch anim.
        // 4.  if player at (x2,y2) position => shoots + switch anim.
        // 5.  spawn projectile + move it
        // 6.  if projectile hit enemy enemy blinks and health -1
        // 7.  player jumps and shoots again
        // 8.  repeat 5-6
        // 9.  enemy dies
        // 10. spawn + animate explosion

        if (SDL_PollEvent(event))
        {
            if (event->type == SDL_EVENT_QUIT)
                isRunning = false;
        }
    }

    SDL_Quit();

    return EXIT_SUCCESS;
}
