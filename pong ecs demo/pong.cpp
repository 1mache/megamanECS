#include "pong.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>
using namespace std;

#include "bagel.h"
using namespace bagel;

namespace mta
{
	// Helper for making a Drawable from a sprite-sheet rectangle.
	// The source rectangle says "which pixels from pong.png", and size says
	// "how big to draw that sprite on screen after TEX_SCALE".
	constexpr Drawable Pong::makeDrawable(SDL_FRect part)
	{
		return Drawable{{part}, {part.w * TEX_SCALE, part.h * TEX_SCALE}};
	}

	// Put the ball back in the middle and give it a new velocity.
	// This is called at game start and after the ball crosses a score sensor.
	void Pong::resetBall(b2BodyId ballBody) const
	{
		// xs is always positive, so the ball starts by moving to the right.
		// ys is between roughly -0.5 and 0.5, so the vertical angle changes.
		float xs = 0.5f + SDL_randf() / 2;
		float ys = SDL_randf() - 0.5f;

		// Box2D positions are in physics units, not pixels, so divide by BOX_SCALE.
		b2Body_SetTransform(ballBody, {WIN_W / 2 / BOX_SCALE, WIN_H / 2 / BOX_SCALE}, b2MakeRot(0));
		b2Body_SetLinearVelocity(ballBody, {xs * 30, ys * 30});
	}

	// Score system:
	// - It does not look for a normal ECS component mask.
	// - It asks Box2D which sensor contacts ended this physics step.
	// - In this demo, crossing a left/right sensor means the ball should reset.
	void Pong::score_system() const
	{
		const auto se = b2World_GetSensorEvents(box);
		for (int i = 0; i < se.endCount; ++i)
		{
			// visitorShapeId is the moving shape that touched the score sensor.
			// Its body is the ball body, so reset that body.
			const b2BodyId b = b2Shape_GetBody(se.endEvents[i].visitorShapeId);
			resetBall(b);
		}
	}

	// Movement system:
	// - Searches for entities that have Intent + Collider.
	// - Reads Intent, which was written by input_system().
	// - Writes velocity into the Box2D body stored in Collider.
	// This means the paddles move because their physics bodies move.
	void Pong::move_system() const
	{
		static const Mask mask = MaskBuilder()
									 .set<Intent>()
									 .set<Collider>()
									 .build();

		for (Entity e = Entity::first(); !e.eof(); e.next())
		{
			if (e.test(mask))
			{
				const auto &i = e.get<Intent>();
				const auto &c = e.get<Collider>();

				// Negative y is up in SDL screen coordinates, positive y is down.
				// If neither key is pressed, the paddle stops.
				const float f = i.up ? -30 : i.down ? 30
													: 0;
				b2Body_SetLinearVelocity(c.b, {0, f});
			}
		}
	}

	// Input system:
	// - Searches for entities that have Keys + Intent.
	// - Reads the current keyboard state using the scancodes in Keys.
	// - Writes simple booleans into Intent.
	// This separates "which key was pressed" from "what the entity wants to do".
	void Pong::input_system() const
	{
		static const Mask mask = MaskBuilder()
									 .set<Keys>()
									 .set<Intent>()
									 .build();

		SDL_PumpEvents();
		const bool *keys = SDL_GetKeyboardState(nullptr);

		// Only entities with both Keys and Intent are controlled by the keyboard.
		// In this demo that means the two paddles, not the ball or the center dots.
		for (Entity e = Entity::first(); !e.eof(); e.next())
		{
			if (e.test(mask))
			{
				const auto &k = e.get<Keys>();
				auto &i = e.get<Intent>();

				i.up = keys[k.up];
				i.down = keys[k.down];
			}
		}
	}

	// Box system:
	// - Advances the Box2D physics world by one fixed step.
	// - Searches for entities with Transform + Collider.
	// - Reads the Box2D body position/angle.
	// - Writes the matching ECS Transform used later by draw_system().
	// This keeps physics as the source of truth for moving objects.
	void Pong::box_system() const
	{
		static constexpr float BOX_STEP = 1.f / FPS;
		static const Mask mask = MaskBuilder()
									 .set<Transform>()
									 .set<Collider>()
									 .build();

		b2World_Step(box, BOX_STEP, 4);

		// After physics moves bodies, copy those positions back into ECS data.
		// Drawing does not talk to Box2D directly; it only reads Transform.
		for (Entity e = Entity::first(); !e.eof(); e.next())
		{
			if (e.test(mask))
			{
				const auto t = b2Body_GetTransform(e.get<Collider>().b);
				e.get<Transform>() = {
					{t.p.x * BOX_SCALE, t.p.y * BOX_SCALE},
					RAD_TO_DEG * b2Rot_GetAngle(t.q)};
			}
		}
	}

	// Draw system:
	// - Searches for entities with Transform + Drawable.
	// - Reads where the entity is and which sprite rectangle it uses.
	// - Sends draw commands to SDL.
	// It does not know about keyboard input or physics.
	void Pong::draw_system() const
	{
		static const Mask mask = MaskBuilder()
									 .set<Transform>()
									 .set<Drawable>()
									 .build();

		for (Entity e = Entity::first(); !e.eof(); e.next())
		{
			if (e.test(mask))
			{
				const auto &t = e.get<Transform>();
				const auto &d = e.get<Drawable>();

				// Transform.p is treated as the center of the entity.
				// SDL wants the top-left corner, so subtract half the sprite size.
				SDL_FRect dest = {
					t.p.x - d.size.x / 2,
					t.p.y - d.size.y / 2,
					d.size.x, d.size.y};

				SDL_RenderTextureRotated(
					ren, tex, &d.part, &dest, static_cast<double>(t.a),
					nullptr, SDL_FLIP_NONE);
			}
		}
	}

	Pong::Pong()
	{
		constexpr float WIN_H_MID = WIN_H / 2.f;
		constexpr float WIN_W_MID = WIN_W / 2.f;

		// Constructor overview:
		// 1. Create SDL objects for drawing.
		// 2. Create a Box2D world for physics.
		// 3. Create ECS entities by attaching components to ids.
		// The comments below point out which objects are ECS entities and which
		// objects exist only inside Box2D.

		// initialize SDL for video (drawing to screen)
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			cout << SDL_GetError() << endl;
			return;
		}

		// create game window
		if (!SDL_CreateWindowAndRenderer(
				"Pong!", WIN_W, WIN_H, 0, &win, &ren))
		{
			cout << SDL_GetError() << endl;
			return;
		}

		// load spritesheet image
		SDL_Surface *surf = IMG_Load("res/pong.png");
		if (surf == nullptr)
		{
			cout << SDL_GetError() << endl;
			return;
		}

		// create texture from spritesheet and destroy orig image
		tex = SDL_CreateTextureFromSurface(ren, surf);
		if (tex == nullptr)
		{
			cout << SDL_GetError() << endl;
			return;
		}
		SDL_DestroySurface(surf);

		// seed randomizer and set default clear color
		SDL_srand(static_cast<Uint64>(time(nullptr)));
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

		// create Box2D world
		b2WorldDef worldDef = b2DefaultWorldDef();
		worldDef.gravity = {0, 0};
		box = b2CreateWorld(&worldDef);
		if (!b2World_IsValid(box))
		{
			cout << "Failed creating Box2D world" << endl;
			return;
		}

		// create center dots (only drawings)
		// Each dot is an entity with Transform + Drawable.
		// It has no Collider, so physics systems ignore it.
		for (float i = 10; i < WIN_H; i += 24)
		{
			Entity::create().addAll(
				Transform{{WIN_W_MID, i}, 0},
				makeDrawable({296, 20, 24, 24}));
		}

		// create top & bottom walls (no entities - they just block movement)
		// These walls are only Box2D bodies. They are not drawn and never need ECS data.
		b2BodyDef wallDef = b2DefaultBodyDef();
		wallDef.type = b2_staticBody;
		wallDef.position = {WIN_W_MID / BOX_SCALE, -1};
		// top wall
		b2ShapeDef wallShapeDef = b2DefaultShapeDef();
		wallShapeDef.density = 1;
		b2Polygon wallPoly = b2MakeBox(WIN_W_MID / BOX_SCALE, 1);
		b2BodyId wallBody = b2CreateBody(box, &wallDef);
		b2CreatePolygonShape(wallBody, &wallShapeDef, &wallPoly);
		// bottom wall
		wallDef.position.y = WIN_H / BOX_SCALE + 1;
		wallBody = b2CreateBody(box, &wallDef);
		b2CreatePolygonShape(wallBody, &wallShapeDef, &wallPoly);

		// create left and right score sensors (entities for detecting collisions)
		// Sensors are Box2D shapes that report overlap but do not push the ball.
		// They are also represented by ECS entities with Scorer, mainly to show
		// how a game object can store a handle to a physics shape.
		wallShapeDef.isSensor = true;
		wallShapeDef.enableSensorEvents = true;
		// left "wall" score sensor
		wallPoly = b2MakeBox(5, WIN_H_MID / BOX_SCALE);
		wallDef.position = {-5, WIN_H_MID / BOX_SCALE};
		wallBody = b2CreateBody(box, &wallDef);
		b2ShapeId wallShape = b2CreatePolygonShape(wallBody, &wallShapeDef, &wallPoly);
		Entity::create().add(Scorer{wallShape});
		// right "wall" score sensor
		wallDef.position = {WIN_W / BOX_SCALE + 5, WIN_H_MID / BOX_SCALE};
		wallBody = b2CreateBody(box, &wallDef);
		wallShape = b2CreatePolygonShape(wallBody, &wallShapeDef, &wallPoly);
		Entity::create().add(Scorer{wallShape});

		// create ball Box2D body def & circle shape
		// The ball is dynamic because Box2D controls its motion and collisions.
		b2BodyDef ballDef = b2DefaultBodyDef();
		ballDef.type = b2_dynamicBody;
		ballDef.position = {WIN_W_MID / BOX_SCALE, WIN_H_MID / BOX_SCALE};
		b2ShapeDef ballShapeDef = b2DefaultShapeDef();
		ballShapeDef.enableSensorEvents = true;
		ballShapeDef.density = 1;
		ballShapeDef.material.friction = 0;
		// Restitution is bounciness. A value above 1 keeps the demo lively.
		ballShapeDef.material.restitution = 1.2f;
		b2Circle ballCircle = {{0, 0}, 76 * TEX_SCALE / BOX_SCALE / 2};
		// create ball Box2D body and add to entity
		b2BodyId ballBody = b2CreateBody(box, &ballDef);
		b2CreateCircleShape(ballBody, &ballShapeDef, &ballCircle);
		resetBall(ballBody);

		// create ball entity
		// The ball entity has Transform + Drawable + Collider.
		// box_system updates its Transform from Collider, and draw_system draws it.
		Entity::create().addAll(
			Transform{{WIN_W_MID, WIN_H_MID}, 0},
			makeDrawable({404, 580, 76, 76}),
			Collider{ballBody});
		// set link from ball body to ball entity (user-data)
		// b2Body_SetUserData(ballBody, new ent_type{ball});

		// create paddles for both users (no AI)
		b2BodyDef padDef = b2DefaultBodyDef();
		// Kinematic bodies are moved by setting velocity, not by forces.
		// That fits player-controlled paddles.
		padDef.type = b2_kinematicBody; // only moves by input
		padDef.position = {50 / BOX_SCALE, WIN_H_MID / BOX_SCALE};
		b2BodyId padBody = b2CreateBody(box, &padDef);

		b2ShapeDef padShapeDef = b2DefaultShapeDef();
		padShapeDef.density = 1;
		b2Polygon padPoly = b2MakeBox(64 * TEX_SCALE / BOX_SCALE / 2, 532 * TEX_SCALE / BOX_SCALE / 2);
		b2CreatePolygonShape(padBody, &padShapeDef, &padPoly);

		// first paddle (left player)
		// This entity has every component needed by the loop:
		// Keys + Intent for input, Collider for physics, Transform + Drawable for drawing.
		Entity::create().addAll(
			Transform{{42, WIN_H_MID}, 0},
			makeDrawable({360, 4, 64, 532}),
			Collider{padBody},
			Intent{},
			Keys{SDL_SCANCODE_W, SDL_SCANCODE_S});

		// second paddle (right player)
		// Same component shape as the left paddle, but with different keys and position.
		padDef.position = {(WIN_W - 50) / BOX_SCALE, WIN_H_MID / BOX_SCALE};
		padBody = b2CreateBody(box, &padDef);
		padPoly = b2MakeBox(64 * TEX_SCALE / BOX_SCALE / 2, 532 * TEX_SCALE / BOX_SCALE / 2);
		b2CreatePolygonShape(padBody, &padShapeDef, &padPoly);
		// entity & components for 2nd paddle
		Entity::create().addAll(
			Transform{{WIN_W - 42, WIN_H_MID}, 0},
			makeDrawable({456, 4, 64, 532}),
			Collider{padBody},
			Intent{},
			Keys{SDL_SCANCODE_UP, SDL_SCANCODE_DOWN});
	}

	Pong::~Pong()
	{
		// The class owns these external resources, so it releases them here.
		// ECS data is static inside bagel for this small demo and is not manually freed.
		if (b2World_IsValid(box))
			b2DestroyWorld(box);
		if (tex != nullptr)
			SDL_DestroyTexture(tex);
		if (ren != nullptr)
			SDL_DestroyRenderer(ren);
		if (win != nullptr)
			SDL_DestroyWindow(win);

		SDL_Quit();
	}

	void Pong::run()
	{
		auto start = SDL_GetTicks();
		bool quit = false;

		while (!quit)
		{
			// System order matters:
			// input_system writes Intent from the keyboard.
			// move_system reads Intent and writes Box2D velocities.
			// box_system steps physics and writes ECS Transforms.
			// score_system reacts to sensor events created by the physics step.
			input_system();
			move_system();
			box_system();
			score_system();

			// Drawing is last so it uses the newest Transform values.
			SDL_RenderClear(ren);
			draw_system();
			SDL_RenderPresent(ren);

			// Simple frame limiter. If this frame was faster than 1/60 second,
			// sleep for the remaining time.
			const auto end = SDL_GetTicks();
			if (end - start < GAME_FRAME)
			{
				SDL_Delay(static_cast<Uint32>(GAME_FRAME - (end - start)));
			}
			start += GAME_FRAME;

			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				// SDL_EVENT_QUIT is the window close button.
				// Escape is a keyboard shortcut to close the demo.
				if ((e.type == SDL_EVENT_QUIT) ||
					((e.type == SDL_EVENT_KEY_DOWN) && (e.key.scancode == SDL_SCANCODE_ESCAPE)))
				{
					quit = true;
				}
			}
		}
	}
}
