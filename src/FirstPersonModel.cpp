#include "FirstPersonModel.h"
#include "Logger.h"
#include "Offsets.h"
#include "CameraAPI.h"
#include <cstring>

namespace FirstPersonModel {

FirstPersonModelMod::FirstPersonModelMod() = default;

FirstPersonModelMod::~FirstPersonModelMod() {
    if (m_initialized) {
        shutdown();
    }
}

FirstPersonModelMod& FirstPersonModelMod::getInstance() {
    static FirstPersonModelMod instance;
    return instance;
}

bool FirstPersonModelMod::initialize() {
    if (m_initialized) {
        LOGD("FirstPersonModel already initialized");
        return true;
    }
    
    LOGI("Initializing FirstPersonModel v1.0.0");
    
    // Initialize offset manager
    auto& offsetManager = OffsetManager::getInstance();
    if (!offsetManager.initialize()) {
        LOGE("Failed to initialize offset manager");
        return false;
    }
    
    // Initialize camera API
    auto& cameraAPI = CameraAPI::getInstance();
    if (!cameraAPI.initialize(nullptr)) {
        LOGE("Failed to initialize camera API");
        return false;
    }
    
    m_initialized = true;
    LOGI("FirstPersonModel initialized successfully");
    return true;
}

void FirstPersonModelMod::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    LOGI("Shutting down FirstPersonModel");
    
    // Restore any hooks
    auto& cameraAPI = CameraAPI::getInstance();
    cameraAPI.shutdown();
    
    m_initialized = false;
    LOGI("FirstPersonModel shutdown complete");
}

bool FirstPersonModelMod::shouldRenderModelInFirstPerson() const {
    if (!m_enabled) {
        return false;
    }
    
    auto& cameraAPI = CameraAPI::getInstance();
    return cameraAPI.isValid() && cameraAPI.isFirstPerson();
}

void FirstPersonModelMod::onCameraRender() {
    if (!m_enabled || !m_initialized) {
        return;
    }
    
    auto& cameraAPI = CameraAPI::getInstance();
    if (!cameraAPI.isValid()) {
        return;
    }
    
    m_isFirstPerson = cameraAPI.isFirstPerson();
    
    if (m_isFirstPerson) {
        // Apply camera offset to avoid clipping with player model
        cameraAPI.applyFirstPersonOffset(m_cameraOffset.x, m_cameraOffset.y, m_cameraOffset.z);
    }
}

void FirstPersonModelMod::onPlayerRender() {
    if (!m_enabled || !m_initialized) {
        return;
    }
    
    // The player model rendering is handled by hooks in PlayerRenderer.cpp
    // This function is called from the player render hook
}

} // namespace FirstPersonModel
