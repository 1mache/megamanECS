#include "GlobalData.h"
#include "MTransform.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <iostream>

constexpr float GRAVITY_SCALE = 10.f;
constexpr int   FPS = 60;
constexpr float TIMESTEP = 1.f / FPS;
constexpr int   SUBSTEPS = 4;

constexpr int             WIN_WIDTH = 720;
constexpr int             WIN_HEIGHT = 540;
constexpr float           WIN_WIDTHF = WIN_WIDTH;
constexpr float           WIN_HEIGHTF = WIN_HEIGHT;
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

    SDL_SetWindowPosition(window,
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);

    return true;
}

int main()
{
    using namespace megaman;

    SDL_Window*   window{};
    SDL_Renderer* renderer{};
    if (!createWindowAndRenderer("Box2D Test", window, renderer))
        return EXIT_FAILURE;

    GlobalData::setWindow(window);
    GlobalData::setRenderer(renderer);

    auto ptm = GlobalData::PTM;

    Transform gtransform = {.x = WIN_WIDTH / ptm / 2.f,
                            .y = 2.f,
                            .w = WIN_WIDTH / ptm,
                            .h = 0.3f,
                            .rot = 0.f};

    Transform boxTransform = {.x = (WIN_WIDTHF / ptm / 2.f),
                              .y = (WIN_HEIGHTF / ptm / 2.f),
                              .w = 0.5f,
                              .h = 0.5f,
                              .rot = 0.f};

    b2WorldDef wd = b2DefaultWorldDef();
    wd.gravity = {0.f, -1.f * GRAVITY_SCALE};
    b2WorldId wId = b2CreateWorld(&wd);

    b2BodyDef groundDef = b2DefaultBodyDef();
    groundDef.position = transformGetb2Pos(gtransform);
    b2BodyId groundId = b2CreateBody(wId, &groundDef);

    auto       gscaleb2 = transformGetb2Scale(gtransform);
    b2Polygon  groundBox = b2MakeBox(gscaleb2.x, gscaleb2.y);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    b2BodyDef boxDef = b2DefaultBodyDef();
    boxDef.type = b2_dynamicBody;
    boxDef.position = transformGetb2Pos(boxTransform);
    b2BodyId boxId = b2CreateBody(wId, &boxDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.f;
    shapeDef.material.restitution = 0.5f;
    shapeDef.enableContactEvents = true;

    b2Polygon box = b2MakeSquare(transformGetb2Scale(boxTransform).x);
    b2CreatePolygonShape(boxId, &shapeDef, &box);

    auto boxPosVec = b2Body_GetPosition(boxId);
    // std::cout << "Box start pos: (" << boxPosVec.x << ',' << yflip(boxPosVec.y)
    //           << ")\n";
    // std::cout << "Ground pos: (" << gpos[0] << ',' << gpos[1] << ")\n";

    auto groundRect = transform2Frect(gtransform);

    bool isRunning = true;
    while (isRunning)
    {
        Uint64 frameStart = SDL_GetTicks();

        SDL_RenderClear(renderer);

        b2World_Step(wId, TIMESTEP, SUBSTEPS);
        boxPosVec = b2Body_GetPosition(boxId);
        transformInjectb2Pos(boxTransform, boxPosVec);

        auto boxRect = transform2Frect(boxTransform);

        SDL_SetRenderDrawColor(renderer, 0, 255, 120, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &groundRect);

        SDL_SetRenderDrawColor(renderer, 200, 150, 150, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &boxRect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderPresent(renderer);

        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED ||
                (event.type == SDL_EVENT_KEY_UP &&
                 event.key.key == SDLK_ESCAPE))
                isRunning = false;
        }

        Uint64           elapsed = SDL_GetTicks() - frameStart;
        constexpr Uint64 frameTime = static_cast<Uint64>(TIMESTEP * 1000);
        if (elapsed < frameTime)
            SDL_Delay(static_cast<Uint32>(frameTime - elapsed));
    }

    b2DestroyWorld(wId);
}
