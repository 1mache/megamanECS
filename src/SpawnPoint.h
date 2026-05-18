/**
 * @file SpawnPoint.h
 * @brief World-coordinate spawn/checkpoint descriptor loaded from the TMX map.
 */
#pragma once
#include <string_view>

namespace megaman
{

/**
 * @brief A single point-object read from the spawn layer.
 *
 * Player entries double as checkpoints; enemy entries carry patrol-destination
 * info where applicable (Ptrol type only).
 */
struct SpawnPoint
{
    enum Type
    {
        Player = 0,
        Ptrol,
        PtrolDest,
        Lockster,
        Boss,
        count
    };

    static constexpr const char* typenames[Type::count]{"player",
                                                        "ptrol",
                                                        "ptrold",
                                                        "lockster",
                                                        "boss"};
    Type                         type{Type::count}; // invalid value by default
    float                        x{};
    float                        y{};           // world coords, meters, Y-up
    float                        patrolDestX{}; // only used for Ptrol type
};

std::string_view spawnTypeToString(SpawnPoint::Type type);

} // namespace megaman
