#pragma once
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include "bagel.h"

namespace mta
{
	// Components are plain data structs.
	// An entity becomes "a paddle", "the ball", or "a center dot" only by
	// the set of components attached to its id.

	// Where the entity is on the screen.
	// p is the center point in SDL pixels, and a is the drawing angle in degrees.
	using Transform = struct { SDL_FPoint p; float a; };

	// What part of the sprite sheet to draw, and how large it should appear.
	// part is the rectangle inside res/pong.png. size is the final screen size.
	using Drawable = struct { SDL_FRect part; SDL_FPoint size; };

	// What the player wants this frame.
	// Input writes this component, and movement reads it. This keeps keyboard
	// details out of the movement system.
	using Intent = struct { bool up, down; };

	// Which keyboard buttons control this entity.
	// The left paddle gets W/S, and the right paddle gets Up/Down.
	using Keys = struct { SDL_Scancode up, down; };

	// Link from an ECS entity to a Box2D physics body.
	// The ECS side stores game meaning; Box2D stores physics state.
	using Collider = struct { b2BodyId b; };

	// Link to a Box2D sensor shape used for scoring.
	// These shapes do not block the ball. They only report that the ball crossed them.
	using Scorer = struct { b2ShapeId s; };
}

// By default, bagel stores components sparsely by entity id.
// Keys and Intent are only used by paddles, so packed storage keeps only the
// entities that actually have those components close together in memory.
template <> struct bagel::Storage<mta::Keys> final : NoInstance {
	using type = PackedStorage<mta::Keys>;
};
template <> struct bagel::Storage<mta::Intent> final : NoInstance {
	using type = PackedStorage<mta::Intent>;
};

namespace mta {
	// Pong owns the SDL window/renderer, the sprite texture, and the Box2D world.
	// The game behavior itself is split into systems such as input, movement,
	// physics sync, scoring, and drawing.
	class Pong
	{
	public:
		Pong();
		~Pong(); // not virtual because this class is not meant to be inherited from.

		void run();

		// The constructor can fail while creating SDL or Box2D objects.
		// valid() lets main decide whether it is safe to start the game loop.
		bool valid() const { return b2World_IsValid(box); }
	private:
		// Window size in screen pixels.
		static constexpr int	WIN_W = 800;
		static constexpr int	WIN_H = 600;

		// The loop tries to run one update every GAME_FRAME milliseconds.
		static constexpr int	FPS = 60;
		static constexpr Uint64	GAME_FRAME = 1000/FPS;

		// Box2D gives angles in radians, while SDL drawing expects degrees.
		static constexpr float	RAD_TO_DEG = 57.2958f;

		// Texture art is larger than this demo needs, so drawn sprites are scaled down.
		static constexpr float TEX_SCALE = 0.25f;

		// Box2D physics uses meters-like units. The renderer uses pixels.
		// BOX_SCALE converts between the two so physics numbers stay reasonable.
		static constexpr float BOX_SCALE = 10.f;

		// Systems are the active part of ECS. Each system searches for entities
		// with the component combination it understands.
		void box_system() const;
		void input_system() const;
		void move_system() const;
		void score_system() const;
		void draw_system() const;
		void resetBall(b2BodyId) const;

		static constexpr Drawable makeDrawable(SDL_FRect part);

		// SDL objects used for drawing.
		SDL_Texture*		tex = nullptr;
		SDL_Renderer*		ren = nullptr;
		SDL_Window*			win = nullptr;

		// Box2D world used for collisions and movement.
		b2WorldId			box = b2_nullWorldId;
	};
}
