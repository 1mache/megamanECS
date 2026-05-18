#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "Utils.h"
#include <iostream>
#include <vector>

constexpr int             WIN_WIDTH = 720;
constexpr int             WIN_HEIGHT = 540;
constexpr SDL_WindowFlags WIN_FLAGS = 0;

constexpr int FRAME_DELAY_MS = 100;

// =============== MEGAMAN =================
constexpr int MEGAMAN_SPRITE_DIM[] = {28, 28};
enum class Megaman
{ // in order start frame of each anim
    RUN = 0,
    SHOOTRUN = 4,
    IDLE = 8,
    SHOOT = 9,
    JUMP = 10
};
// frame count of each animation
constexpr int MEGAMAN_RUN_FRAME_COUNT = 4;
[[maybe_unused]]
constexpr int MEGAMAN_SHOOTRUN_FRAME_COUNT = 4;
constexpr int MEGAMAN_IDLE_FRAME_COUNT = 1;
[[maybe_unused]]
constexpr int MEGAMAN_SHOOT_FRAME_COUNT = 1;
constexpr int MEGAMAN_JUMP_FRAME_COUNT = 1;

// movement relevant constants
constexpr float MEGAMAN_START_POS[] = {8, 162};
constexpr float BLOCK_POS[] = {128.f, 160.f};
constexpr float MEGAMAN_RUN_SPEED = 2.f;
constexpr float MEGAMAN_GROUND_Y = MEGAMAN_START_POS[1];
constexpr float MEGAMAN_BLOCK_Y = BLOCK_POS[1] - 28.f;
constexpr int   JUMP_DURATION_FRAMES = 20;
constexpr float JUMP_PEAK_OFFSET = 80.f;

enum class MegamanState
{
    IDLE,
    RUN,
    SHOOTRUN,
    SHOOT,
    JUMP
};
// =============== ENEMY =================
constexpr int ENEMY_SPRITE_DIM[] = {24, 24};

enum class Enemy
{ // one animation, for consistency left the enum
    HOVER = 0
};
constexpr int ENEMY_HOVER_FRAME_COUNT = 2;
// movement:
constexpr int   ENEMY_START_POS[] = {250, 67};
constexpr float ENEMY_PATROL_RIGHT_X = ENEMY_START_POS[0];
constexpr float ENEMY_PATROL_LEFT_X = ENEMY_PATROL_RIGHT_X - 100.f;
constexpr float ENEMY_SPEED = 1.5f;
constexpr int   ENEMY_BLINK_FRAMES = 10;
constexpr int   ENEMY_BLINK_PERIOD = 3;

// =============== EXPLOSION ==============-
constexpr int EXPLOSION_SPRITE_DIM[] = {22, 24};
constexpr int EXPLOSION_FRAME_COUNT = 3;

// ================ SHOT =================
constexpr int   SHOT_SPRITE_DIM[] = {8, 8};
constexpr float SHOT_SPEED = 10.f;

// ============ UTILITY  ============
struct SpriteSheet
{
    SDL_Texture* texture{};
    float        w{};  // texture width
    float        h{};  // texture height
    float        sw{}; // single sprite width
    float        sh{}; // single sprite height
};

struct Animation
{
    SpriteSheet* spriteSheet{};
    int          startFrameId{};
    int          frameCount{};
};

struct MegamanAnimations
{
    Animation* run{};
    Animation* shootRun{};
    Animation* idle{};
    Animation* shoot{};
    Animation* jump{};
};

struct MegamanInput
{
    bool moveLeft{};
    bool moveRight{};
    bool jumpPressed{};
    bool shootPressed{};
};

struct MegamanRuntime
{
    SDL_FRect    dstRect{};
    SDL_FlipMode flip{SDL_FLIP_NONE};
    MegamanState state{MegamanState::IDLE};
    int          animFrame{};
    int          jumpFrame{};
    int          shootAnimTimer{};
    bool         isJumping{};
    float        worldWidth{};
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

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

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

// ============== INIT FUNCTIONS ==================
SpriteSheet createBackgroundSpriteSheet(SDL_Window* window, SDL_Renderer* renderer)
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

SpriteSheet createExplosionSpriteSheet(SDL_Window* window, SDL_Renderer* renderer)
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

// =================== RENDERING FUNCTIONS ======================

void renderBackground(const SpriteSheet& bg, SDL_Renderer* renderer)
{
    SDL_FRect dstRect{0, 0, bg.w, bg.h};
    SDL_RenderTexture(renderer, bg.texture, nullptr, &dstRect);
}

SDL_FRect getNextAnimationFrame(const Animation& anim, int frameIndex)
{
    // start of the animation on x axis
    float x =
        static_cast<float>(anim.startFrameId + frameIndex) * anim.spriteSheet->sw;
    massert(x + anim.spriteSheet->sw <= anim.spriteSheet->w);

    SDL_FRect srcRect{x, 0, anim.spriteSheet->sw, anim.spriteSheet->sh};
    return srcRect;
}


void renderAnimationFrame(const Animation& anim,
                          int              frameIndex,
                          const SDL_FRect& dstRect,
                          SDL_Renderer*    renderer,
                          SDL_FlipMode     flipMode = SDL_FLIP_NONE)
{
    SDL_FRect srcRect = getNextAnimationFrame(anim, frameIndex);
    SDL_RenderTextureRotated(renderer,
                             anim.spriteSheet->texture,
                             &srcRect,
                             &dstRect,
                             0,
                             nullptr,
                             flipMode);
}

float clampMegamanX(float x, float width, float worldWidth)
{
    if (x < 0.f)
        return 0.f;
    if (x + width > worldWidth)
        return worldWidth - width;
    return x;
}

float getMegamanSupportY(const SDL_FRect& megamanDstRect)
{
    const float megamanCenterX = megamanDstRect.x + megamanDstRect.w * 0.5f;
    return megamanCenterX >= BLOCK_POS[0] ? MEGAMAN_BLOCK_Y : MEGAMAN_GROUND_Y;
}

void setMegamanState(MegamanRuntime& megaman, MegamanState nextState)
{
    if (megaman.state == nextState)
        return;

    megaman.state = nextState;
    megaman.animFrame = 0;
}

void advanceMegamanAnimation(MegamanRuntime& megaman, const Animation& currentAnim)
{
    megaman.animFrame = (megaman.animFrame + 1) % currentAnim.frameCount;
}

void handleMegamanIdle(MegamanRuntime&          megaman,
                       const MegamanAnimations& animations,
                       const Animation*&        currentAnim)
{
    megaman.dstRect.y = getMegamanSupportY(megaman.dstRect);

    if (megaman.shootAnimTimer > 0)
    {
        setMegamanState(megaman, MegamanState::SHOOT);
        currentAnim = animations.shoot;
        return;
    }

    setMegamanState(megaman, MegamanState::IDLE);
    currentAnim = animations.idle;
}

void handleMegamanRunRight(MegamanRuntime&          megaman,
                           const MegamanAnimations& animations,
                           const Animation*&        currentAnim)
{
    megaman.flip = SDL_FLIP_NONE;
    megaman.dstRect.x = clampMegamanX(megaman.dstRect.x + MEGAMAN_RUN_SPEED,
                                      megaman.dstRect.w,
                                      megaman.worldWidth);
    megaman.dstRect.y = getMegamanSupportY(megaman.dstRect);

    if (megaman.shootAnimTimer > 0)
    {
        setMegamanState(megaman, MegamanState::SHOOTRUN);
        currentAnim = animations.shootRun;
        return;
    }

    setMegamanState(megaman, MegamanState::RUN);
    currentAnim = animations.run;
}

void handleMegamanRunLeft(MegamanRuntime&          megaman,
                          const MegamanAnimations& animations,
                          const Animation*&        currentAnim)
{
    megaman.flip = SDL_FLIP_HORIZONTAL;
    megaman.dstRect.x = clampMegamanX(megaman.dstRect.x - MEGAMAN_RUN_SPEED,
                                      megaman.dstRect.w,
                                      megaman.worldWidth);
    megaman.dstRect.y = getMegamanSupportY(megaman.dstRect);

    if (megaman.shootAnimTimer > 0)
    {
        setMegamanState(megaman, MegamanState::SHOOTRUN);
        currentAnim = animations.shootRun;
        return;
    }

    setMegamanState(megaman, MegamanState::RUN);
    currentAnim = animations.run;
}

void startMegamanJump(MegamanRuntime& megaman)
{
    if (megaman.isJumping)
        return;

    megaman.isJumping = true;
    megaman.jumpFrame = 0;
    setMegamanState(megaman, MegamanState::JUMP);
}

void handleMegamanJump(MegamanRuntime&          megaman,
                       const MegamanInput&      input,
                       const MegamanAnimations& animations,
                       const Animation*&        currentAnim)
{
    if (input.moveLeft == input.moveRight)
    {
        // keep facing the current direction while doing a straight-up jump
    }
    else if (input.moveLeft)
    {
        megaman.flip = SDL_FLIP_HORIZONTAL;
        megaman.dstRect.x = clampMegamanX(megaman.dstRect.x - MEGAMAN_RUN_SPEED,
                                          megaman.dstRect.w,
                                          megaman.worldWidth);
    }
    else
    {
        megaman.flip = SDL_FLIP_NONE;
        megaman.dstRect.x = clampMegamanX(megaman.dstRect.x + MEGAMAN_RUN_SPEED,
                                          megaman.dstRect.w,
                                          megaman.worldWidth);
    }

    const float t = static_cast<float>(megaman.jumpFrame) / JUMP_DURATION_FRAMES;
    const float supportY = getMegamanSupportY(megaman.dstRect);
    megaman.dstRect.y = supportY - JUMP_PEAK_OFFSET * SDL_sinf(SDL_PI_F * t);
    ++megaman.jumpFrame;

    if (megaman.jumpFrame >= JUMP_DURATION_FRAMES)
    {
        megaman.isJumping = false;
        megaman.jumpFrame = 0;
        megaman.dstRect.y = getMegamanSupportY(megaman.dstRect);
    }

    setMegamanState(megaman, MegamanState::JUMP);
    currentAnim = animations.jump;
}

void handleMegamanShoot(MegamanRuntime&    megaman,
                        bool&              shotActive,
                        SDL_FRect&         shotDstRect,
                        float&             shotVelocityX,
                        const SpriteSheet& shot)
{
    if (shotActive)
        return;

    megaman.shootAnimTimer = 4;
    shotActive = true;
    shotVelocityX = megaman.flip == SDL_FLIP_HORIZONTAL ? -SHOT_SPEED : SHOT_SPEED;

    if (shotVelocityX > 0.f)
        shotDstRect.x = megaman.dstRect.x + megaman.dstRect.w;
    else
        shotDstRect.x = megaman.dstRect.x - shot.sw;

    shotDstRect.y = megaman.dstRect.y + megaman.dstRect.h * 0.5f - shot.sh * 0.5f;
}

int main()
{
    SDL_Window*   window{};
    SDL_Renderer* renderer{};

    if (!createWindowAndRenderer("Megaman Mockup", window, renderer))
        return EXIT_FAILURE;

    bool isRunning = true;


    SpriteSheet bg = createBackgroundSpriteSheet(window, renderer);
    // save scale factor used to scale bg image to window
    const float scaleFactor = WIN_WIDTH / bg.w;
    // scale everything by the same factor to preserve sprite aspect ratio
    SDL_SetRenderScale(renderer, scaleFactor, scaleFactor);

    SpriteSheet megaman = createMegamanSpriteSheet(window, renderer);
    Animation   megamanRunAnim{&megaman,
                               static_cast<int>(Megaman::RUN),
                               MEGAMAN_RUN_FRAME_COUNT};
    Animation   megamanShootRunAnim{&megaman,
                                    static_cast<int>(Megaman::SHOOTRUN),
                                    MEGAMAN_SHOOTRUN_FRAME_COUNT};
    Animation   megamanIdleAnim{&megaman,
                                static_cast<int>(Megaman::IDLE),
                                MEGAMAN_IDLE_FRAME_COUNT};
    Animation   megamanShootAnim{&megaman,
                                 static_cast<int>(Megaman::SHOOT),
                                 MEGAMAN_SHOOT_FRAME_COUNT};
    Animation   megamanJumpAnim{&megaman,
                                static_cast<int>(Megaman::JUMP),
                                MEGAMAN_JUMP_FRAME_COUNT};

    MegamanAnimations megamanAnimations{&megamanRunAnim,
                                        &megamanShootRunAnim,
                                        &megamanIdleAnim,
                                        &megamanShootAnim,
                                        &megamanJumpAnim};

    MegamanRuntime megamanRuntime{
        {MEGAMAN_START_POS[0], MEGAMAN_START_POS[1], megaman.sw, megaman.sh},
        SDL_FLIP_NONE,
        MegamanState::IDLE,
        0,
        0,
        0,
        false,
        bg.w};
    MegamanInput megamanInput{};

    SpriteSheet enemy = createEnemySpriteSheet(window, renderer);
    Animation   enemyHoverAnim{&enemy,
                               static_cast<int>(Enemy::HOVER),
                               ENEMY_HOVER_FRAME_COUNT};
    SDL_FRect   enemyDstRect{ENEMY_START_POS[0],
                             ENEMY_START_POS[1],
                             enemy.sw,
                             enemy.sh};

    SpriteSheet shot = createShotSpriteSheet(window, renderer);
    Animation   shotFlyAnim{&shot, 0, 1};

    SpriteSheet explosion = createExplosionSpriteSheet(window, renderer);
    Animation   explosionAnim{&explosion, 0, EXPLOSION_FRAME_COUNT};


    int enemyAnimFrame = 0;

    float        enemyDir = 1.f; // +1 right, -1 left
    SDL_FlipMode enemyFlip = SDL_FLIP_NONE;
    bool         enemyAlive = true;
    int          enemyHp = 2;

    bool      shotActive = false;
    SDL_FRect shotDstRect{0, 0, shot.sw, shot.sh};
    float     shotVelocityX = SHOT_SPEED;
    int       enemyBlinkTimer = 0;

    bool      explosionActive = false;
    int       explosionFrame = 0;
    SDL_FRect explosionDstRect{0, 0, explosion.sw, explosion.sh};

    while (isRunning)
    {
        SDL_Event event{};
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                isRunning = false;
                continue;
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_A || event.key.key == SDLK_LEFT)
                    megamanInput.moveLeft = true;
                if (event.key.key == SDLK_D || event.key.key == SDLK_RIGHT)
                    megamanInput.moveRight = true;
                if (!event.key.repeat &&
                    (event.key.key == SDLK_SPACE || event.key.key == SDLK_W ||
                     event.key.key == SDLK_UP))
                    megamanInput.jumpPressed = true;
                if (!event.key.repeat &&
                    (event.key.key == SDLK_J || event.key.key == SDLK_X))
                    megamanInput.shootPressed = true;
            }

            if (event.type == SDL_EVENT_KEY_UP)
            {
                if (event.key.key == SDLK_A || event.key.key == SDLK_LEFT)
                    megamanInput.moveLeft = false;
                if (event.key.key == SDLK_D || event.key.key == SDLK_RIGHT)
                    megamanInput.moveRight = false;
            }
        }

        if (megamanInput.jumpPressed)
            startMegamanJump(megamanRuntime);

        if (megamanInput.shootPressed)
            handleMegamanShoot(megamanRuntime,
                               shotActive,
                               shotDstRect,
                               shotVelocityX,
                               shot);

        const Animation* megamanCurrentAnim = megamanAnimations.idle;
        if (megamanRuntime.isJumping)
        {
            handleMegamanJump(megamanRuntime,
                              megamanInput,
                              megamanAnimations,
                              megamanCurrentAnim);
        }
        else if (megamanInput.moveLeft && !megamanInput.moveRight)
        {
            handleMegamanRunLeft(megamanRuntime,
                                 megamanAnimations,
                                 megamanCurrentAnim);
        }
        else if (megamanInput.moveRight && !megamanInput.moveLeft)
        {
            handleMegamanRunRight(megamanRuntime,
                                  megamanAnimations,
                                  megamanCurrentAnim);
        }
        else
        {
            handleMegamanIdle(megamanRuntime, megamanAnimations, megamanCurrentAnim);
        }

        megamanInput.jumpPressed = false;
        megamanInput.shootPressed = false;

        SDL_RenderClear(renderer);
        renderBackground(bg, renderer);
        renderAnimationFrame(*megamanCurrentAnim,
                             megamanRuntime.animFrame,
                             megamanRuntime.dstRect,
                             renderer,
                             megamanRuntime.flip);
        advanceMegamanAnimation(megamanRuntime, *megamanCurrentAnim);

        if (enemyAlive)
        {
            enemyDstRect.x += enemyDir * ENEMY_SPEED;
            if (enemyDstRect.x >= ENEMY_PATROL_RIGHT_X)
            {
                enemyDir = -1.f;
                enemyFlip = SDL_FLIP_NONE;
            }
            if (enemyDstRect.x <= ENEMY_PATROL_LEFT_X)
            {
                enemyDir = 1.f;
                enemyFlip = SDL_FLIP_HORIZONTAL;
            }
        }
        // simple collision detection lambda
        auto rectsOverlap = [](const SDL_FRect& a, const SDL_FRect& b) {
            return !(a.x + a.w <= b.x || b.x + b.w <= a.x || a.y + a.h <= b.y ||
                     b.y + b.h <= a.y);
        };

        if (shotActive)
        {
            shotDstRect.x += shotVelocityX;
            if (enemyAlive && rectsOverlap(shotDstRect, enemyDstRect))
            {
                shotActive = false;
                --enemyHp;
                if (enemyHp > 0)
                {
                    enemyBlinkTimer = ENEMY_BLINK_FRAMES;
                }
                else
                {
                    enemyAlive = false;
                    // enemy died -> spawn explosion
                    explosionActive = true;
                    explosionFrame = 0;
                    explosionDstRect.x =
                        enemyDstRect.x + enemy.sw * 0.5f - explosion.sw * 0.5f;
                    explosionDstRect.y =
                        enemyDstRect.y + enemy.sh * 0.5f - explosion.sh * 0.5f;
                }
            }
            else if (shotDstRect.x > bg.w || shotDstRect.x + shotDstRect.w < 0.f)
            {
                shotActive = false;
            }
        }

        if (megamanRuntime.shootAnimTimer > 0)
            --megamanRuntime.shootAnimTimer;

        if (enemyBlinkTimer > 0)
            --enemyBlinkTimer;

        // divide timer into BLINK_PERIOD-sized buckets. even bucket = visible, odd = hidden
        const bool enemyVisible =
            enemyAlive && (enemyBlinkTimer == 0 ||
                           (enemyBlinkTimer / ENEMY_BLINK_PERIOD) % 2 == 0);
        if (enemyVisible)
        {
            renderAnimationFrame(enemyHoverAnim,
                                 enemyAnimFrame,
                                 enemyDstRect,
                                 renderer,
                                 enemyFlip);
            enemyAnimFrame = (enemyAnimFrame + 1) % enemyHoverAnim.frameCount;
        }
        if (shotActive)
            renderAnimationFrame(shotFlyAnim, 0, shotDstRect, renderer);
        if (explosionActive)
        {
            if (explosionFrame >= EXPLOSION_FRAME_COUNT)
            {
                explosionActive = false;
            }
            else
            {
                renderAnimationFrame(explosionAnim,
                                     explosionFrame,
                                     explosionDstRect,
                                     renderer);
                ++explosionFrame;
            }
        }
        SDL_RenderPresent(renderer);

        SDL_Delay(FRAME_DELAY_MS);
    }

    destroyResourcesAndQuit(window, renderer);

    return EXIT_SUCCESS;
}