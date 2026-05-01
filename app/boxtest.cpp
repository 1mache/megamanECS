#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <iostream>

constexpr float GRAVITY_SCALE = 10.f;
constexpr int   FPS = 60;
constexpr float TIMESTEP = 1.f / FPS;
constexpr int   SUBSTEPS = 4;

// constexpr float PTM = 30.f;

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

constexpr float yflip(float boxY)
{
    return WIN_HEIGHTF - boxY;
}


int main()
{
    SDL_Window*   window{};
    SDL_Renderer* renderer{};
    if (!createWindowAndRenderer("Box2D Test", window, renderer))
        return EXIT_FAILURE;

    const float gpos[2]{0.f, 350.f};
    const float gscale[2]{WIN_WIDTH, 10.f};
    [[maybe_unused]]
    const float gcenter[2]{(gpos[0] + gscale[0] / 2),
                           (gpos[1] + gscale[1] / 2)};
    float       boxPos[2]{(WIN_WIDTHF / 2.f), (WIN_HEIGHTF / 2.f)};
    const float boxScale[2]{10.f, 10.f};
    float       boxCenter[2]{boxPos[0] + boxScale[0] / 2,
                             boxPos[1] + boxScale[1] / 2};

    SDL_FRect groundRect{gpos[0], gpos[1], gscale[0], gscale[1]};
    SDL_FRect bodyRect{boxPos[0], boxPos[1], boxScale[0], boxScale[1]};

    b2WorldDef wd = b2DefaultWorldDef();
    wd.gravity = {0.f, -1.f * GRAVITY_SCALE};
    b2WorldId wId = b2CreateWorld(&wd);

    b2BodyDef groundDef = b2DefaultBodyDef();
    groundDef.position = {gcenter[0], yflip(gcenter[1])};
    b2BodyId groundId = b2CreateBody(wId, &groundDef);

    b2Polygon  groundBox = b2MakeBox(gscale[0] / 2.f, gscale[1] / 2.f);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    b2BodyDef boxDef = b2DefaultBodyDef();
    boxDef.type = b2_dynamicBody;
    boxDef.position = {boxCenter[0], yflip(boxCenter[1])};
    b2BodyId boxId = b2CreateBody(wId, &boxDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.f;
    shapeDef.material.restitution = 0.5f;
    shapeDef.enableContactEvents = true;

    b2Polygon box = b2MakeSquare(boxScale[0] / 2.f);
    b2CreatePolygonShape(boxId, &shapeDef, &box);

    auto boxPosVec = b2Body_GetPosition(boxId);
    std::cout << "Box start pos: (" << boxPosVec.x << ',' << yflip(boxPosVec.y)
              << ")\n";
    std::cout << "Ground pos: (" << gpos[0] << ',' << gpos[1] << ")\n";


    bool isRunning = true;
    while (isRunning)
    {
        Uint64 frameStart = SDL_GetTicks();

        SDL_RenderClear(renderer);

        b2World_Step(wId, TIMESTEP, SUBSTEPS);
        boxPosVec = b2Body_GetPosition(boxId);

        bodyRect.x = boxPosVec.x - boxScale[0] / 2.f;
        bodyRect.y = yflip(boxPosVec.y) - boxScale[1] / 2.f;

        SDL_SetRenderDrawColor(renderer, 0, 255, 120, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &groundRect);

        SDL_SetRenderDrawColor(renderer, 200, 150, 150, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &bodyRect);

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
