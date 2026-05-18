/**
 * @file MegamanGame.h
 * @brief Top-level game object: owns SDL/Box2D resources, the scene, and drives the main loop.
 */
#pragma once

#include "Scene.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace megaman
{
/**
 * @brief Initialises the engine and runs the game.
 *
 * The constructor performs all one-time setup (SDL, Box2D, textures, entities).
 * Call valid() before run() to confirm initialisation succeeded.
 */
class MegamanGame
{
public:
    /**
     * @brief Initialises SDL, creates window/renderer, loads all textures,
     *        creates the Box2D world, loads the scene, and spawns all entities.
     *
     * If any step fails the relevant pointer stays null; valid() will return false
     * and run() should not be called.
     */
    MegamanGame();

    /** @brief Destroys all SDL textures, the renderer, the window, the Box2D world, and quits SDL. */
    ~MegamanGame();

    /** @brief Returns true if the window and renderer were created successfully. */
    bool valid() const
    {
        return _win != nullptr && _ren != nullptr;
    }

    /**
     * @brief Fixed-timestep main loop. Runs until the window is closed or Escape is pressed.
     *
     * System call order per frame:
     *  1. inputSystem         — SDL keyboard → Intent
     *  2. aiSystem            — enemy AI → Intent
     *  3. bossSystem          — boss AI → Intent
     *  4. jumpSystem          — ground detection, coyote, buffer, variable gravity
     *  5. movementSystem      — Intent → Box2D velocity, fall-off-world kill
     *  6. checkpointSystem    — update last checkpoint from player position
     *  7. shootingSystem      — Intent + cooldown → spawn projectile
     *  8. collisionSystem     — Box2D step + contact dispatch → DamageIntent
     *  9. damageSystem        — DamageIntent → Health, i-frame start, freeze enemies
     * 10. healthSystem        — death handling, destroy queue
     * 11. respawnSystem       — player/enemy respawn
     * 12. *AnimSystems        — update animation state + tickAnim → RenderFrame
     * 13. Scene clamping      — clamp camera to map bounds
     * 14. projectileCullSystem — destroy out-of-view bullets
     * 15. Scene/drawSystem/hudSystem — render everything
     */
    void run();

private:
    static constexpr int         HP = 3;
    static constexpr int         WIN_W = 720;
    static constexpr int         WIN_H = 540;
    static constexpr int         FPS = 60;
    static constexpr Uint64      GAME_FRAME = 1000 / FPS;
    static constexpr const char* MAP_PATH = "res/map/map.tmx";
    static constexpr const char* PLAYER_TEXTURE_PATH = "res/player.png";

    SDL_Window*   _win = nullptr;
    SDL_Renderer* _ren = nullptr;
    SDL_Texture*  _tex = nullptr;
    SDL_Texture*  _enemyTex = nullptr;
    SDL_Texture*  _locksterTex = nullptr;
    SDL_Texture*  _bossTex = nullptr;
    SDL_Texture*  _shotTex = nullptr;
    SDL_Texture*  _explosionTex = nullptr;
    SDL_Texture*  _heartTex = nullptr;

    b2WorldId _boxWorld = b2_nullWorldId;
    Scene     _scene{MAP_PATH};
};
} // namespace megaman
