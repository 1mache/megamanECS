#include "MapCollisionLayer.h"

#include "GlobalData.h"
#include "Megaman.h"

#include <tmxlite/ObjectGroup.hpp>

#include "Utils.h"

namespace megaman
{

bool MapCollisionLayer::create(const tmx::Map& map, std::uint32_t layerIndex)
{
    const auto& layers = map.getLayers();
    massert(layerIndex < layers.size(), "Incorrect layer index, out of bounds");
    massert(layers[layerIndex]->getType() == tmx::Layer::Type::Object,
            "Layer index does not point to an object layer");

    const auto& group = layers[layerIndex]->getLayerAs<tmx::ObjectGroup>();

    constexpr float invPTM = 1.f / GlobalData::PTM;
    const float     mapHM =
        static_cast<float>(map.getTileCount().y * map.getTileSize().y) * invPTM;

    for (const auto& obj : group.getObjects())
    {
        if (obj.getShape() != tmx::Object::Shape::Rectangle)
            continue;

        const auto& aabb = obj.getAABB();

        float halfW = aabb.width * 0.5f * invPTM;
        float halfH = aabb.height * 0.5f * invPTM;
        float cx    = (aabb.left + aabb.width * 0.5f) * invPTM;
        // tmxlite Y is down from map top; flip to world Y-up
        float cy = mapHM - (aabb.top + aabb.height * 0.5f) * invPTM;

        _rects.push_back({.x = cx, .y = cy, .w = halfW, .h = halfH});
    }

    return isValid();
}

void MapCollisionLayer::createBodies(b2WorldId worldId)
{
    massert(isValid(), "Must call create() successfully before createBodies()");
    for (const auto& rect : _rects)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type      = b2_staticBody;
        bodyDef.position  = {rect.x, rect.y};
        bodyDef.userData  = reinterpret_cast<void*>(static_cast<uintptr_t>(-1));
        b2BodyId bodyId   = b2CreateBody(worldId, &bodyDef);

        b2Polygon  groundBox               = b2MakeBox(rect.w, rect.h);
        b2ShapeDef groundShapeDef          = b2DefaultShapeDef();
        groundShapeDef.filter.categoryBits = CAT_WORLD;
        groundShapeDef.filter.maskBits =
            CAT_PLAYER | CAT_ENEMY | CAT_PLAYER_BULLET | CAT_ENEMY_BULLET;
        b2CreatePolygonShape(bodyId, &groundShapeDef, &groundBox);
    }
}

} // namespace megaman
