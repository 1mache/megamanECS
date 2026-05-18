/**
 * @file CameraData.h
 * @brief World-space camera position used by all rendering systems.
 */
#pragma once

namespace megaman
{
/** @brief Camera position in world coordinates (meters, Y-up). */
struct CameraData
{
    float posX{}, posY{};
};
} // namespace megaman
