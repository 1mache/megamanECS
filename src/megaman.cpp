#include "megaman.h"

namespace megaman
{
ent_type createPlayer(float x, float y, int hp)
{
    ent_type ent = bagel::World::createEntity();

    bagel::Storage<Drawable>::type::add(ent, {.texture = nullptr});
    bagel::Storage<Transform>::type::add(ent, {.posX = x, .posY = y});
    bagel::Storage<Movement>::type::add(ent, {.mass = 1});
    bagel::Storage<Collision>::type::add(ent, {});
    bagel::Storage<Health>::type::add(ent, {.points = hp});
    bagel::Storage<Input>::type::add(ent, {});
    bagel::Storage<Weapon>::type::add(ent, {});
    bagel::Storage<Sound>::type::add(ent, {});

    return ent;
}

ent_type createEnemy(float x, float y, int hp)
{
    ent_type ent = bagel::World::createEntity();

    bagel::Storage<Drawable>::type::add(ent, {.texture = nullptr});
    bagel::Storage<Transform>::type::add(ent, {.posX = x, .posY = y});
    bagel::Storage<Movement>::type::add(ent, {.mass = 1});
    bagel::Storage<Collision>::type::add(ent, {});
    bagel::Storage<Health>::type::add(ent, {.points = hp});
    bagel::Storage<Enemy>::type::add(ent, {});
    bagel::Storage<AI>::type::add(ent, {.state = -1});
    bagel::Storage<Sound>::type::add(ent, {});

    return ent;
}

ent_type createBoss(float x, float y, int hp)
{
    ent_type ent = bagel::World::createEntity();

    bagel::Storage<Drawable>::type::add(ent, {.texture = nullptr});
    bagel::Storage<Transform>::type::add(ent, {.posX = x, .posY = y});
    bagel::Storage<Movement>::type::add(ent, {.mass = 1});
    bagel::Storage<Collision>::type::add(ent, {});
    bagel::Storage<Health>::type::add(ent, {.points = hp});
    bagel::Storage<Enemy>::type::add(ent, {});
    bagel::Storage<AI>::type::add(ent, {.state = -1});
    // based on boss type
    bagel::Storage<Weapon>::type::add(ent, {.projectileType = -1});
    bagel::Storage<Sound>::type::add(ent, {});

    return ent;
}

ent_type createPlatform(float x, float y, bool isMoving)
{
    ent_type ent = bagel::World::createEntity();

    bagel::Storage<Transform>::type::add(ent, {.posX = x, .posY = y});
    bagel::Storage<Collision>::type::add(ent, {});
    if (isMoving)
        bagel::Storage<Movement>::type::add(ent, {.mass = 0});

    return ent;
}

ent_type createProjectile(float x, float y, float velX, float velY)
{
    ent_type ent = bagel::World::createEntity();

    bagel::Storage<Drawable>::type::add(ent, {.texture = nullptr});
    bagel::Storage<Transform>::type::add(ent, {.posX = x, .posY = y});
    bagel::Storage<Movement>::type::add(
        ent,
        {.mass = 1, .velX = velX, .velY = velY});
    bagel::Storage<Collision>::type::add(ent, {});

    return ent;
}

ent_type createTrigger(float x, float y, float width, float height)
{
    ent_type ent = bagel::World::createEntity();

    bagel::Storage<Transform>::type::add(ent, {.posX = x, .posY = y});
    bagel::Storage<Collision>::type::add(ent,
                                         {.width = width, .height = height});

    return ent;
}

ent_type createItem(float x, float y)
{
    ent_type ent = bagel::World::createEntity();

    bagel::Storage<Drawable>::type::add(ent, {.texture = nullptr});
    bagel::Storage<Transform>::type::add(ent, {.posX = x, .posY = y});
    bagel::Storage<Collision>::type::add(ent, {});

    return ent;
}

ent_type createText(float x, float y, const std::string& text)
{
    ent_type ent = bagel::World::createEntity();

    bagel::Storage<Drawable>::type::add(
        ent,
        {.texture = nullptr}); // tie it to the text later somehow
    bagel::Storage<Transform>::type::add(ent, {.posX = x, .posY = y});

    return ent;
}

ent_type createSoundSource(float x, float y, int sound)
{
    ent_type ent = bagel::World::createEntity();

    bagel::Storage<Transform>::type::add(ent, {.posX = x, .posY = y});
    bagel::Storage<Sound>::type::add(ent, {.sound = sound});

    return ent;
}

ent_type createScene(const std::string& mapFilePath)
{
    ent_type ent = bagel::World::createEntity();

    bagel::Storage<Scene>::type::add(ent, {.mapFilePath = mapFilePath});

    return ent;
}
} // namespace megaman
