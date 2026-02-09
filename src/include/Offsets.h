#pragma once

#include <cstdint>
#include <cstddef>

namespace FirstPersonModel {

// Dynamic offset management - allows for version compatibility
class OffsetManager {
public:
    static OffsetManager& getInstance();
    
    bool initialize();
    bool isInitialized() const { return m_initialized; }
    
    // Camera offsets
    size_t getCameraPositionOffset() const { return m_cameraPositionOffset; }
    size_t getCameraRotationOffset() const { return m_cameraRotationOffset; }
    size_t getCameraFovOffset() const { return m_cameraFovOffset; }
    size_t getCameraPerspectiveOffset() const { return m_cameraPerspectiveOffset; }
    
    // Player offsets
    size_t getPlayerPositionOffset() const { return m_playerPositionOffset; }
    size_t getPlayerRotationOffset() const { return m_playerRotationOffset; }
    
    // Function addresses
    void* getCameraRenderAddress() const { return m_cameraRenderAddr; }
    void* getCameraIsFirstPersonAddress() const { return m_cameraIsFirstPersonAddr; }
    void* getPlayerRenderAddress() const { return m_playerRenderAddr; }
    void* getGameRendererRenderLevelAddress() const { return m_gameRendererRenderLevelAddr; }
    
private:
    OffsetManager() = default;
    ~OffsetManager() = default;
    
    bool m_initialized = false;
    
    // Camera offsets
    size_t m_cameraPositionOffset = 0x10;
    size_t m_cameraRotationOffset = 0x20;
    size_t m_cameraFovOffset = 0x30;
    size_t m_cameraPerspectiveOffset = 0x34;
    
    // Player offsets
    size_t m_playerPositionOffset = 0x2B0;
    size_t m_playerRotationOffset = 0x2C0;
    
    // Function addresses
    void* m_cameraRenderAddr = nullptr;
    void* m_cameraIsFirstPersonAddr = nullptr;
    void* m_playerRenderAddr = nullptr;
    void* m_gameRendererRenderLevelAddr = nullptr;
    
    bool scanSignatures();
    bool resolveOffsets();
};

// Version-specific offsets (fallback values)
namespace VersionOffsets {
    // Minecraft 1.20.x
    struct v1_20 {
        static constexpr size_t CAMERA_POS = 0x10;
        static constexpr size_t CAMERA_ROT = 0x20;
        static constexpr size_t CAMERA_FOV = 0x30;
        static constexpr size_t PLAYER_POS = 0x2B0;
    };
    
    // Minecraft 1.21.x
    struct v1_21 {
        static constexpr size_t CAMERA_POS = 0x18;
        static constexpr size_t CAMERA_ROT = 0x28;
        static constexpr size_t CAMERA_FOV = 0x38;
        static constexpr size_t PLAYER_POS = 0x2C0;
    };
}

} // namespace FirstPersonModel
