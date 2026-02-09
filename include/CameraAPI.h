#pragma once

#include "Logger.h"
#include <cstdint>
#include <cstddef>

namespace FirstPersonModel {

// Camera API interface for hooking
class ICameraAPI {
public:
    virtual ~ICameraAPI() = default;
    
    // Position
    virtual void getPosition(float* x, float* y, float* z) const = 0;
    virtual void setPosition(float x, float y, float z) = 0;
    
    // Rotation (pitch, yaw, roll)
    virtual void getRotation(float* pitch, float* yaw, float* roll) const = 0;
    virtual void setRotation(float pitch, float yaw, float roll) = 0;
    
    // Perspective check
    virtual bool isFirstPerson() const = 0;
    virtual bool isThirdPerson() const = 0;
    virtual int getPerspectiveMode() const = 0; // 0 = first person, 1 = third person back, 2 = third person front
    
    // FOV
    virtual float getFov() const = 0;
    virtual void setFov(float fov) = 0;
};

// Camera implementation wrapper
class CameraAPI {
public:
    static CameraAPI& getInstance();
    
    bool initialize(void* gameInstance);
    void shutdown();
    
    // Direct hooks
    void* getCameraPtr() const { return m_cameraPtr; }
    bool isValid() const { return m_cameraPtr != nullptr; }
    
    // Camera operations
    void applyFirstPersonOffset(float x, float y, float z);
    void restoreOriginalPosition();
    
    bool isFirstPerson() const;
    
    // VTable manipulation
    bool hookRenderFunction();
    bool unhookRenderFunction();
    
private:
    CameraAPI();
    ~CameraAPI();
    
    void* m_cameraPtr = nullptr;
    void* m_originalRender = nullptr;
    void* m_hookedRender = nullptr;
    
    float m_originalPos[3] = {0, 0, 0};
    bool m_positionModified = false;
};

// VTable indices for Camera class (may need adjustment per version)
constexpr int CAMERA_VTABLE_GET_POSITION = 3;
constexpr int CAMERA_VTABLE_SET_POSITION = 4;
constexpr int CAMERA_VTABLE_GET_ROTATION = 5;
constexpr int CAMERA_VTABLE_SET_ROTATION = 6;
constexpr int CAMERA_VTABLE_IS_FIRST_PERSON = 12;
constexpr int CAMERA_VTABLE_RENDER = 25;

} // namespace FirstPersonModel
