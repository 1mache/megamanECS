#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <cassert>
#include <iostream>
#include <vector>

constexpr int             WIN_WIDTH = 720;
constexpr int             WIN_HEIGHT = 540;
constexpr SDL_WindowFlags WIN_FLAGS = 0;

constexpr int FRAME_DELAY_MS = 100;

[[maybe_unused]]
constexpr int MEGAMAN_SPRITE_DIM[] = {28, 28};
[[maybe_unused]]
constexpr int ENEMY_SPRITE_DIM[] = {22, 24};
[[maybe_unused]]
constexpr int EXPLOSION_SPRITE_DIM[] = {22, 24};

enum class Megaman
{ // in order start frame of each anim
    RUN = 0,
    SHOOTRUN = 4,
    IDLE = 8,
    SHOOT = 9,
    JUMP = 10
};
constexpr int MEGAMAN_RUN_FRAME_COUNT = 4;
[[maybe_unused]]
constexpr int MEGAMAN_SHOOTRUN_FRAME_COUNT = 4;
[[maybe_unused]]
constexpr int MEGAMAN_IDLE_FRAME_COUNT = 1;
[[maybe_unused]]
constexpr int MEGAMAN_SHOOT_FRAME_COUNT = 1;
[[maybe_unused]]
constexpr int MEGAMAN_JUMP_FRAME_COUNT = 1;

constexpr float MEGAMAN_START_POS[] = {8, 162};

enum class Enemy
{ // in order
    HOVER = 0
};
[[maybe_unused]]
constexpr int   ENEMY_HOVER_FRAME_COUNT = 2;
constexpr int   ENEMY_START_POS[] = {250, 65};
constexpr float ENEMY_PATROL_LEFT_X = 150.f;
constexpr float ENEMY_PATROL_RIGHT_X = 250.f;
constexpr float ENEMY_SPEED = 1.0f;

enum class Explosion
{ // in order
    EXPLODE = 0
};
[[maybe_unused]]
constexpr int EXPLOSION_FRAME_COUNT = 3;

[[maybe_unused]]
constexpr int SHOT_FRAME_COUNT = 1;

enum class Shot
{
    FLY = 0
};

constexpr int SHOT_SPRITE_DIM[] = {8, 6};

struct SpriteSheet
{
    SDL_Texture* texture{};
    float        w{};  // texture width
    float        h{};  // texture height
    float        sw{}; // sprite width
    float        sh{}; // sprite height
};

struct Animation
{
    SpriteSheet* spriteSheet{};
    int          startFrame{};
    int          frameCount{};
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
        std::exit(EXIT_FAILURE);
    }
    if (!window)
    {
        std::cerr << "Window creation error : " << SDL_GetError() << std::endl;
        SDL_Quit();
        std::exit(EXIT_FAILURE);
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

SpriteSheet createBackgroundSpriteSheet(SDL_Window*   window,
                                        SDL_Renderer* renderer)
{
    SpriteSheet bg{};
    bg.texture = createTexture("res/area.png", renderer);
    if (!bg.texture)
    {
        destroyResourcesAndQuit(window, renderer);
        std::exit(EXIT_FAILURE);
    }
    SDL_GetTextureSize(bg.texture, &bg.w, &bg.h);
    bg.sw = bg.w; //single image
    bg.sh = bg.h;
    return bg;
}

SpriteSheet createMegamanSpriteSheet(SDL_Window* window, SDL_Renderer* renderer)
{
    SpriteSheet megaman{};
    megaman.texture = createTexture("res/player.png", renderer);
    if (!megaman.texture)
    {
        destroyResourcesAndQuit(window, renderer);
        std::exit(EXIT_FAILURE);
    }
    SDL_GetTextureSize(megaman.texture, &megaman.w, &megaman.h);
    megaman.sw = MEGAMAN_SPRITE_DIM[0];
    megaman.sh = MEGAMAN_SPRITE_DIM[1];
    return megaman;
}

SpriteSheet createEnemySpriteSheet(SDL_Window* window, SDL_Renderer* renderer)
{
    SpriteSheet enemy{};
    enemy.texture = createTexture("res/enemy.png", renderer);
    if (!enemy.texture)
    {
        destroyResourcesAndQuit(window, renderer);
        std::exit(EXIT_FAILURE);
    }
    SDL_GetTextureSize(enemy.texture, &enemy.w, &enemy.h);
    enemy.sw = ENEMY_SPRITE_DIM[0];
    enemy.sh = ENEMY_SPRITE_DIM[1];
    return enemy;
}

SpriteSheet createExplosionSpriteSheet(SDL_Window*   window,
                                       SDL_Renderer* renderer)
{
    SpriteSheet explosion{};
    explosion.texture = createTexture("res/explosion.png", renderer);
    if (!explosion.texture)
    {
        destroyResourcesAndQuit(window, renderer);
        std::exit(EXIT_FAILURE);
    }
    SDL_GetTextureSize(explosion.texture, &explosion.w, &explosion.h);
    explosion.sw = EXPLOSION_SPRITE_DIM[0];
    explosion.sh = EXPLOSION_SPRITE_DIM[1];
    return explosion;
}

SpriteSheet createShotSpriteSheet(SDL_Window* window, SDL_Renderer* renderer)
{
    SpriteSheet shot{};
    shot.texture = createTexture("res/shot.png", renderer);
    if (!shot.texture)
    {
        destroyResourcesAndQuit(window, renderer);
        std::exit(EXIT_FAILURE);
    }
    SDL_GetTextureSize(shot.texture, &shot.w, &shot.h);
    shot.sw = SHOT_SPRITE_DIM[0];
    shot.sh = SHOT_SPRITE_DIM[1];
    return shot;
}

// Rendering functions:

void renderBackground(const SpriteSheet& bg, SDL_Renderer* renderer)
{
    SDL_FRect dstRect{0, 0, bg.w, bg.h};
    SDL_RenderTexture(renderer, bg.texture, nullptr, &dstRect);
}

SDL_FRect getNextAnimationFrame(const Animation& anim, int frameIndex)
{
    // start of the animation on x axis
    float x =
        static_cast<float>(anim.startFrame + frameIndex) * anim.spriteSheet->sw;
    assert(x + anim.spriteSheet->sw <= anim.spriteSheet->w);

    SDL_FRect srcRect{x, 0, anim.spriteSheet->sw, anim.spriteSheet->sh};
    return srcRect;
}

void renderMegaman(const Animation& anim,
                   int&             frameIndex,
                   const SDL_FRect& dstRect,
                   SDL_Renderer*    renderer)
{
    SDL_FRect srcRect = getNextAnimationFrame(anim, frameIndex);
    frameIndex = (frameIndex + 1) % anim.frameCount; // loop animation
    SDL_RenderTexture(renderer, anim.spriteSheet->texture, &srcRect, &dstRect);
}

void renderEnemy(const Animation& anim,
                 int&             frameIndex,
                 const SDL_FRect& dstRect,
                 SDL_Renderer*    renderer)
{
    SDL_FRect srcRect = getNextAnimationFrame(anim, frameIndex);
    frameIndex = (frameIndex + 1) % anim.frameCount;
    SDL_RenderTexture(renderer, anim.spriteSheet->texture, &srcRect, &dstRect);
}

void renderShot(const Animation& anim,
                int&             frameIndex,
                const SDL_FRect& dstRect,
                SDL_Renderer*    renderer)
{
    SDL_FRect srcRect = getNextAnimationFrame(anim, frameIndex);
    frameIndex = (frameIndex + 1) % anim.frameCount;
    SDL_RenderTexture(renderer, anim.spriteSheet->texture, &srcRect, &dstRect);
}

void renderExplosion(const Animation& anim,
                     int&             frameIndex,
                     const SDL_FRect& dstRect,
                     SDL_Renderer*    renderer)
{
    SDL_FRect srcRect = getNextAnimationFrame(anim, frameIndex);
    // NOTE: caller controls frameIndex (no wrap) so explosion plays once
    SDL_RenderTexture(renderer, anim.spriteSheet->texture, &srcRect, &dstRect);
}

int main()
{
    SDL_Window*   window{};
    SDL_Renderer* renderer{};

    if (!createWindowAndRenderer("Megaman Mockup", window, renderer))
        return EXIT_FAILURE;

    bool isRunning = true;


    SpriteSheet bg = createBackgroundSpriteSheet(window, renderer);
    const float scaleFactor = WIN_WIDTH / bg.w;
    // scale everything by the same factor to preserve sprite aspect ratio
    SDL_SetRenderScale(renderer, scaleFactor, scaleFactor);

    SpriteSheet megaman = createMegamanSpriteSheet(window, renderer);
    Animation   megamanRunAnim{&megaman,
                               static_cast<int>(Megaman::RUN),
                               MEGAMAN_RUN_FRAME_COUNT};
    SDL_FRect   megamanDstRect{MEGAMAN_START_POS[0],
                               MEGAMAN_START_POS[1],
                               megaman.sw,
                               megaman.sh};

    SpriteSheet enemy = createEnemySpriteSheet(window, renderer);
    Animation   enemyHoverAnim{&enemy,
                               static_cast<int>(Enemy::HOVER),
                               ENEMY_HOVER_FRAME_COUNT};
    SDL_FRect   enemyDstRect{ENEMY_START_POS[0],
                             ENEMY_START_POS[1],
                             enemy.sw,
                             enemy.sh};

    SpriteSheet                shot = createShotSpriteSheet(window, renderer);
    [[maybe_unused]] Animation shotFlyAnim{&shot,
                                           static_cast<int>(Shot::FLY),
                                           SHOT_FRAME_COUNT};

    SpriteSheet explosion = createExplosionSpriteSheet(window, renderer);
    [[maybe_unused]] Animation explosionAnim{
        &explosion,
        static_cast<int>(Explosion::EXPLODE),
        EXPLOSION_FRAME_COUNT};


    // 4.init ground collider
    int megamanAnimFrame = 0;
    int enemyAnimFrame = 0;

    float enemyDir = 1.f; // +1 right, -1 left
    bool  enemyAlive = true;
    int   enemyHp = 2;

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

        SDL_RenderClear(renderer);
        renderBackground(bg, renderer);
        renderMegaman(megamanRunAnim,
                      megamanAnimFrame,
                      megamanDstRect,
                      renderer);
        megamanDstRect.x += 2.f; // move right
        if (enemyAlive)
        {
            enemyDstRect.x += enemyDir * ENEMY_SPEED;
            if (enemyDstRect.x >= ENEMY_PATROL_RIGHT_X)
                enemyDir = -1.f;
            if (enemyDstRect.x <= ENEMY_PATROL_LEFT_X)
                enemyDir = 1.f;
        }
        (void)enemyHp;
        if (enemyAlive)
            renderEnemy(enemyHoverAnim, enemyAnimFrame, enemyDstRect, renderer);
        SDL_RenderPresent(renderer);

        SDL_Delay(FRAME_DELAY_MS);

        SDL_Event event{};
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                isRunning = false;
            // else if (event.type == SDL_EVENT_MOUSE_MOTION)
            // {
            //     megamanDstRect.x = event.motion.x / scaleFactor;
            //     megamanDstRect.y = event.motion.y / scaleFactor;
            //     std::cout << "Mouse at (" << megamanDstRect.x << ", "
            //               << megamanDstRect.y << ")\n";
            // }
        }
    }

    SDL_Quit();

    return EXIT_SUCCESS;
}
