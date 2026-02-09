#include "CameraAPI.h"
#include "Logger.h"
#include "Offsets.h"
#include "hook/HookManager.h"
#include "Signatures.h"
#include <cstring>
#include <dlfcn.h>

namespace FirstPersonModel {

// Function pointer types for original functions
using CameraRenderOriginal = void(*)(void* camera, void* renderParams);
using CameraIsFirstPersonOriginal = bool(*)(void* camera);

// Hook functions
static void* g_cameraRenderOriginal = nullptr;
static void* g_cameraIsFirstPersonOriginal = nullptr;

void hookedCameraRender(void* camera, void* renderParams) {
    auto& mod = FirstPersonModelMod::getInstance();
    auto& cameraAPI = CameraAPI::getInstance();
    
    // Call original function
    if (g_cameraRenderOriginal) {
        reinterpret_cast<CameraRenderOriginal>(g_cameraRenderOriginal)(camera, renderParams);
    }
    
    // Apply our modifications
    if (camera) {
        cameraAPI.onCameraRender();
        mod.onCameraRender();
    }
}

bool hookedCameraIsFirstPerson(void* camera) {
    if (g_cameraIsFirstPersonOriginal && camera) {
        return reinterpret_cast<CameraIsFirstPersonOriginal>(g_cameraIsFirstPersonOriginal)(camera);
    }
    return false;
}

CameraAPI::CameraAPI() = default;

CameraAPI::~CameraAPI() {
    shutdown();
}

CameraAPI& CameraAPI::getInstance() {
    static CameraAPI instance;
    return instance;
}

bool CameraAPI::initialize(void* gameInstance) {
    if (m_cameraPtr != nullptr) {
        LOGD("CameraAPI already initialized");
        return true;
    }
    
    LOGI("Initializing CameraAPI");
    
    // Get function addresses from offset manager
    auto& offsetManager = OffsetManager::getInstance();
    
    // Hook camera render function
    void* cameraRenderAddr = offsetManager.getCameraRenderAddress();
    if (cameraRenderAddr) {
        auto& hookManager = HookManager::getInstance();
        if (hookManager.hook(cameraRenderAddr, 
                             reinterpret_cast<void*>(hookedCameraRender),
                             reinterpret_cast<void**>(&g_cameraRenderOriginal))) {
            LOGI("Camera render function hooked at %p", cameraRenderAddr);
        } else {
            LOGE("Failed to hook camera render function");
        }
    }
    
    // Hook isFirstPerson function
    void* isFirstPersonAddr = offsetManager.getCameraIsFirstPersonAddress();
    if (isFirstPersonAddr) {
        auto& hookManager = HookManager::getInstance();
        if (hookManager.hook(isFirstPersonAddr,
                             reinterpret_cast<void*>(hookedCameraIsFirstPerson),
                             reinterpret_cast<void**>(&g_cameraIsFirstPersonOriginal))) {
            LOGI("Camera isFirstPerson function hooked at %p", isFirstPersonAddr);
        }
    }
    
    LOGI("CameraAPI initialized");
    return true;
}

void CameraAPI::shutdown() {
    if (m_cameraPtr == nullptr) {
        return;
    }
    
    LOGI("Shutting down CameraAPI");
    
    // Unhook functions
    auto& hookManager = HookManager::getInstance();
    
    if (g_cameraRenderOriginal) {
        hookManager.unhookFunction(g_cameraRenderOriginal);
        g_cameraRenderOriginal = nullptr;
    }
    
    if (g_cameraIsFirstPersonOriginal) {
        hookManager.unhookFunction(g_cameraIsFirstPersonOriginal);
        g_cameraIsFirstPersonOriginal = nullptr;
    }
    
    m_cameraPtr = nullptr;
    LOGI("CameraAPI shutdown complete");
}

void CameraAPI::applyFirstPersonOffset(float x, float y, float z) {
    if (!m_cameraPtr) {
        return;
    }
    
    auto& offsetManager = OffsetManager::getInstance();
    
    // Read current position
    size_t posOffset = offsetManager.getCameraPositionOffset();
    float* posX = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(m_cameraPtr) + posOffset);
    float* posY = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(m_cameraPtr) + posOffset + sizeof(float));
    float* posZ = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(m_cameraPtr) + posOffset + 2 * sizeof(float));
    
    if (!m_positionModified) {
        m_originalPos[0] = *posX;
        m_originalPos[1] = *posY;
        m_originalPos[2] = *posZ;
        m_positionModified = true;
    }
    
    // Apply offset
    *posX = m_originalPos[0] + x;
    *posY = m_originalPos[1] + y;
    *posZ = m_originalPos[2] + z;
}

void CameraAPI::restoreOriginalPosition() {
    if (!m_cameraPtr || !m_positionModified) {
        return;
    }
    
    auto& offsetManager = OffsetManager::getInstance();
    
    size_t posOffset = offsetManager.getCameraPositionOffset();
    float* posX = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(m_cameraPtr) + posOffset);
    float* posY = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(m_cameraPtr) + posOffset + sizeof(float));
    float* posZ = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(m_cameraPtr) + posOffset + 2 * sizeof(float));
    
    *posX = m_originalPos[0];
    *posY = m_originalPos[1];
    *posZ = m_originalPos[2];
    
    m_positionModified = false;
}

bool CameraAPI::isFirstPerson() const {
    if (!m_cameraPtr) {
        return false;
    }
    
    auto& offsetManager = OffsetManager::getInstance();
    
    size_t perspectiveOffset = offsetManager.getCameraPerspectiveOffset();
    int* perspectiveMode = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(m_cameraPtr) + perspectiveOffset);
    
    return *perspectiveMode == 0; // 0 = first person
}

void CameraAPI::onCameraRender() {
    // This is called from the hooked camera render function
    // We can apply additional modifications here if needed
}

} // namespace FirstPersonModel
