#pragma once
#include <string_view>

namespace megaman
{

struct SpawnPoint
{
    enum Type
    {
        Player = 0,
        Ptrol,
        Lockster,
        Flask,
        count
    };

    static constexpr const char* typenames[Type::count]{"player",
                                                        "ptrol",
                                                        "lockster",
                                                        "flask"};
    Type                         type{Type::count}; // invalid value by default
    float                        x{};
    float                        y{}; // world coords, meters, Y-up
};

std::string_view spawnTypeToString(SpawnPoint::Type type);

} // namespace megaman
