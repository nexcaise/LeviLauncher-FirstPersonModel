#include "Offsets.h"
#include "Logger.h"
#include "Signatures.h"
#include "hook/KittyMemory.h"
#include "hook/KittyScanner.h"
#include <cstring>
#include <dlfcn.h>

namespace FirstPersonModel {

OffsetManager::OffsetManager() = default;

OffsetManager::~OffsetManager() = default;

OffsetManager& OffsetManager::getInstance() {
    static OffsetManager instance;
    return instance;
}

bool OffsetManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    LOGI("Initializing Offset Manager");
    
    // Wait for Minecraft library to be loaded
    const char* minecraftLib = "libminecraftpe.so";
    int attempts = 0;
    const int maxAttempts = 50; // 5 seconds max wait
    
    while (!KittyMemory::isLibraryLoaded(minecraftLib) && attempts < maxAttempts) {
        usleep(100000); // 100ms
        attempts++;
    }
    
    if (!KittyMemory::isLibraryLoaded(minecraftLib)) {
        LOGE("Minecraft library not found after %d attempts", maxAttempts);
        return false;
    }
    
    LOGI("Minecraft library found");
    
    // Scan for signatures
    if (!scanSignatures()) {
        LOGE("Failed to scan signatures");
        return false;
    }
    
    // Resolve offsets based on version
    if (!resolveOffsets()) {
        LOGE("Failed to resolve offsets");
        return false;
    }
    
    m_initialized = true;
    LOGI("Offset Manager initialized successfully");
    return true;
}

bool OffsetManager::scanSignatures() {
    LOGI("Scanning for function signatures");
    
    const char* libName = "libminecraftpe.so";
    
    // Scan for camera render function
    m_cameraRenderAddr = KittyScanner::findIdaPatternInLibrary(
        libName, 
        Signatures::CAMERA_RENDER_ARM64
    );
    
    if (m_cameraRenderAddr) {
        LOGI("Found Camera::render at %p", m_cameraRenderAddr);
    } else {
        LOGD("Camera::render signature not found, will try fallback");
    }
    
    // Scan for isFirstPerson function
    m_cameraIsFirstPersonAddr = KittyScanner::findIdaPatternInLibrary(
        libName,
        Signatures::CAMERA_IS_FIRST_PERSON_ARM64
    );
    
    if (m_cameraIsFirstPersonAddr) {
        LOGI("Found Camera::isFirstPerson at %p", m_cameraIsFirstPersonAddr);
    } else {
        LOGD("Camera::isFirstPerson signature not found, will try fallback");
    }
    
    // Scan for player render function
    m_playerRenderAddr = KittyScanner::findIdaPatternInLibrary(
        libName,
        Signatures::PLAYER_RENDER_ARM64
    );
    
    if (m_playerRenderAddr) {
        LOGI("Found Player::render at %p", m_playerRenderAddr);
    } else {
        LOGD("Player::render signature not found, will try fallback");
    }
    
    // Scan for game renderer
    m_gameRendererRenderLevelAddr = KittyScanner::findIdaPatternInLibrary(
        libName,
        Signatures::GAME_RENDERER_RENDER_LEVEL_ARM64
    );
    
    if (m_gameRendererRenderLevelAddr) {
        LOGI("Found GameRenderer::renderLevel at %p", m_gameRendererRenderLevelAddr);
    }
    
    // If any critical function is missing, we might need fallback methods
    if (!m_cameraRenderAddr && !m_cameraIsFirstPersonAddr) {
        LOGE("Critical camera functions not found");
        return false;
    }
    
    return true;
}

bool OffsetManager::resolveOffsets() {
    LOGI("Resolving offsets for current version");
    
    // Try to determine Minecraft version from library
    void* base = KittyMemory::getLibraryBase("libminecraftpe.so");
    if (!base) {
        LOGE("Failed to get library base");
        return false;
    }
    
    // Default offsets (version 1.20.x)
    // These would be determined by version detection or signature scanning
    m_cameraPositionOffset = Offsets::CAMERA_POSITION_X;
    m_cameraRotationOffset = Offsets::CAMERA_ROTATION_PITCH;
    m_cameraFovOffset = Offsets::CAMERA_FOV;
    m_cameraPerspectiveOffset = Offsets::CAMERA_PERSPECTIVE_MODE;
    m_playerPositionOffset = Offsets::PLAYER_POSITION;
    m_playerRotationOffset = Offsets::PLAYER_ROTATION;
    
    LOGI("Using default offsets (v1.20.x compatible)");
    LOGI("  Camera Position Offset: 0x%zX", m_cameraPositionOffset);
    LOGI("  Camera Rotation Offset: 0x%zX", m_cameraRotationOffset);
    LOGI("  Camera FOV Offset: 0x%zX", m_cameraFovOffset);
    LOGI("  Camera Perspective Offset: 0x%zX", m_cameraPerspectiveOffset);
    
    return true;
}

} // namespace FirstPersonModel
