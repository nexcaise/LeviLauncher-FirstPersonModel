#pragma once

#include <cstdint>
#include <cstddef>

namespace FirstPersonModel {

// Function signatures for pattern scanning
// These patterns help us locate functions across different Minecraft versions

namespace Signatures {

    // Camera::render function - ARM64 pattern
    // Matches: 48 ?? ?? ?? 21 ?? ?? 91 ?? ?? 00 ?? ?? 91 ?? ?? 40 ?? ?? 00 00 ?? ?? 00 94
    constexpr const char* CAMERA_RENDER_ARM64 = 
        "48 ?? ?? ?? 21 ?? ?? 91 ?? ?? 00 ?? ?? 91 ?? ?? 40 ?? ?? 00 00 ?? ?? 00 94";
    
    // Camera::isFirstPerson function
    // Matches: FD 7B ?? A9 F4 4F ?? A9 FD 03 00 91 F3 03 00 ?? ?? ?? 00 ?? ?? ?? 00 94
    constexpr const char* CAMERA_IS_FIRST_PERSON_ARM64 = 
        "FD 7B ?? A9 F4 4F ?? A9 FD 03 00 91 F3 03 00 ?? ?? ?? 00 ?? ?? ?? 00 94";
    
    // Player::render function
    constexpr const char* PLAYER_RENDER_ARM64 = 
        "FF 03 04 D1 FD 7B 0C A9 FA 67 0D A9 F8 5F 0E A9 F6 57 0F A9 F4 4F 10 A9";
    
    // Player::isUsingFirstPersonCamera
    constexpr const char* PLAYER_IS_FIRST_PERSON_ARM64 = 
        "FD 7B ?? A9 F4 4F ?? A9 FD 03 00 91 ?? ?? 00 ?? ?? ?? 00 ?? ?? ?? 00 94";
    
    // GameRenderer::renderLevel
    constexpr const char* GAME_RENDERER_RENDER_LEVEL_ARM64 = 
        "FF 03 05 D1 F4 4F 0F A9 FD 7B 11 A9 FD 43 04 91 94 1A 40 F9";
    
    // LevelRenderer::render
    constexpr const char* LEVEL_RENDERER_RENDER_ARM64 = 
        "FF 03 03 D1 FC 6F 0C A9 FA 67 0D A9 F8 5F 0E A9 F6 57 0F A9 F4 4F 10 A9";
    
    // Options::getFov
    constexpr const char* OPTIONS_GET_FOV_ARM64 = 
        "FD 7B ?? A9 F4 4F ?? A9 FD 03 00 91 08 ?? 40 ?? ?? ?? 00 ?? ?? 00 00 ?? ?? ?? 00 94";
    
} // namespace Signatures

// Memory offsets for class members (relative to class base pointer)
namespace Offsets {
    
    // Camera class offsets
    constexpr int CAMERA_POSITION_X = 0x10;
    constexpr int CAMERA_POSITION_Y = 0x14;
    constexpr int CAMERA_POSITION_Z = 0x18;
    constexpr int CAMERA_ROTATION_PITCH = 0x20;
    constexpr int CAMERA_ROTATION_YAW = 0x24;
    constexpr int CAMERA_ROTATION_ROLL = 0x28;
    constexpr int CAMERA_FOV = 0x30;
    constexpr int CAMERA_PERSPECTIVE_MODE = 0x34;
    
    // Player class offsets
    constexpr int PLAYER_POSITION = 0x2B0;
    constexpr int PLAYER_ROTATION = 0x2C0;
    constexpr int PLAYER_DIMENSION = 0x318;
    constexpr int PLAYER_GAME_MODE = 0xB50;
    
    // LevelRenderer offsets
    constexpr int LEVEL_RENDERER_CAMERA = 0x280;
    
} // namespace Offsets

// Function types for casting
using CameraRenderFunc = void(*)(void* camera, void* renderParams);
using CameraIsFirstPersonFunc = bool(*)(void* camera);
using PlayerRenderFunc = void(*)(void* player, void* renderParams, void* matrix);
using GetFovFunc = float(*)(void* options, float defaultFov);

} // namespace FirstPersonModel
