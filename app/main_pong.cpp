#include <iostream>
using namespace std;

#include "bagel.h"
using namespace bagel;

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

// void run_tests();

class Movement
{
};
class IsPlayer
{
};

template <>
struct bagel::Storage<Movement> final : NoInstance
{
    using type = PackedStorage<Movement>;
};

#include "pong.h"

int main()
{
    PongMockup pm;
    pm.run();
    return 0;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        cout << SDL_GetError() << endl;
        return -1;
    }

    SDL_Window   *win;
    SDL_Renderer *ren;

    if (!SDL_CreateWindowAndRenderer("Bagel", 800, 600, 0, &win, &ren))
    {
        cout << SDL_GetError() << endl;
        return -1;
    }

    SDL_Surface *surf = IMG_Load("res/OSK.jpg");
    if (surf == nullptr)
    {
        cout << SDL_GetError() << endl;
        return -1;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    if (tex == nullptr)
    {
        cout << SDL_GetError() << endl;
        return -1;
    }
    SDL_DestroySurface(surf);

    SDL_FRect src = {0, 0, 100, 100};
    SDL_FRect dst = {50, 50, 200, 200};

    for (int i = 0; i < 1000; ++i)
    {
        SDL_RenderClear(ren);
        SDL_RenderTexture(ren, tex, &src, &dst);
        SDL_RenderPresent(ren);

        dst.x += 1;
        dst.y += 1;
        src.x += 1;
        src.y += 1;

        SDL_Delay(10);
    }

    SDL_Quit();
    //ent_type e = World::createEntity();

    //Storage<int>::type::

    //Storage<Movement>::add(e, Movement{});
    //Movement& m = Storage<Movement>::get(e);

    //Storage<IsPlayer>::add(e, {});


    return 0;
}
