#include "SpawnPoint.h"

std::string_view megaman::spawnTypeToString(SpawnPoint::Type type)
{
    return SpawnPoint::typenames[type];
}