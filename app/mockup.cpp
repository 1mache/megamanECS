#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <iostream>

constexpr int             WIN_WIDTH = 720;
constexpr int             WIN_HEIGHT = 540;
constexpr SDL_WindowFlags WIN_FLAGS = 0;

[[maybe_unused]]
constexpr int MEGAMAN_SPRITE_DIM[] = {28, 28};
[[maybe_unused]]
constexpr int ENEMY_SPRITE_DIM[] = {22, 24};
[[maybe_unused]]
constexpr int EXPLOSION_SPRITE_DIM[] = {22, 24};

struct SpriteSheet
{
    SDL_Texture* texture;
    float        w;  // texture width
    float        h;  // texture height
    float        sw; // sprite width
    float        sh; // sprite height
};

bool createWindowAndRenderer(const char*    title,
                             SDL_Window*&   window,
                             SDL_Renderer*& renderer)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Init error : " << SDL_GetError() << std::endl;
        return false;
    }

    auto success = SDL_CreateWindowAndRenderer(title,
                                               WIN_WIDTH,
                                               WIN_HEIGHT,
                                               WIN_FLAGS,
                                               &window,
                                               &renderer);
    if (!success)
    {
        std::cerr << "Window and renderer creation error : " << SDL_GetError()
                  << '\n';
        SDL_Quit();
        return false;
    }
    if (!window)
    {
        std::cerr << "Window creation error : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    SDL_SetWindowPosition(window,
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);

    return true;
}

SDL_Texture* createTexture(const char* path, SDL_Renderer* renderer)
{
    SDL_Texture* texture = IMG_LoadTexture(renderer, path);
    if (!texture)
    {
        std::cerr << "Texture creation error : " << SDL_GetError() << std::endl;
        return nullptr;
    }
    return texture;
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

    SpriteSheet bg{};
    bg.texture = createTexture("res/area.png", renderer);
    if (!bg.texture)
    {
        destroyResourcesAndQuit(window, renderer);
        return EXIT_FAILURE;
    }

    SDL_GetTextureSize(bg.texture, &bg.w, &bg.h);

    const float scaleFactor = WIN_WIDTH / bg.w;
    // scale everything by the same factor to preserve sprite aspect ratio
    SDL_SetRenderScale(renderer, scaleFactor, scaleFactor);

    // 1.load scene picture
    // 2.init player
    // 3.init enemy
    // 4.init health indication
    // 5.init ground collider


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


        SDL_FRect dstRect{0, 0, bg.w, bg.h};
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, bg.texture, nullptr, &dstRect);
        SDL_RenderPresent(renderer);

        SDL_Event event{};
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                isRunning = false;
        }
    }

    SDL_Quit();

    return EXIT_SUCCESS;
}
