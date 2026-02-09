#include "features/Features.hpp"
#include <cstring>
#include <dlfcn.h>
#include <cstdint>
#include <cmath>
#include "util/IOptions.hpp"
#include "util/IntOption.h"
#include "util/FloatOption.h"
#include "util/OptionID.h"

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
};

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
};

using CameraRenderOriginal = void(*)(void*, void*);
using CameraIsFirstPersonOriginal = bool(*)(void*);
using PlayerRenderOriginal = void(*)(void*, void*, void*);
using GetFovFunc = float(*)(IOptions* options, float defaultFov);

void onCameraRender();
bool inited = false;
bool isFirstPerson();
bool shouldRenderModelInFirstPerson();

bool m_enabled = true;
Vec3 m_cameraOffset = {0.0f, -0.1f, 0.3f};
float m_modelScale = 1.0f;

void* m_cameraPtr = nullptr;

GetFovFunc g_optionsGetFov = nullptr;
CameraRenderOriginal g_cameraRenderOriginal = nullptr;
CameraIsFirstPersonOriginal g_cameraIsFirstPersonOriginal = nullptr;
PlayerRenderOriginal g_playerRenderOriginal = nullptr;

float m_originalPos[3]{};
bool m_positionModified = false;

void hookedCameraRender(void* camera, void* renderParams) {
    m_cameraPtr = camera;
    if (g_cameraRenderOriginal)
        g_cameraRenderOriginal(camera, renderParams);

    if (camera)
        onCameraRender();
}

bool hookedCameraIsFirstPerson(void* camera) {
    if (!camera || !g_cameraIsFirstPersonOriginal)
        return false;

    return g_cameraIsFirstPersonOriginal(camera);
}

float hookedOptionsGetFov(IOptions* options, float defaultFov) {
    if(opt == nullptr) opt = options;
    
    if(!inited) {
    auto& abc = opt->getAllRegisteredOptions();
    FloatOption* gamma = (FloatOption*)abc[(int)OptionID::Gamma].get(); 
    IntOption* view = (IntOption*)abc[(int)OptionID::ViewDistance].get(); 
    view->mValue = 2;
    gamma->mValue = 10000.10000.0f;
    inited = true;
    }
    
    return g_optionsGetFov(options, defaultFov);
}

void hookedPlayerRender(void* player, void* renderParams, void* matrix) {
    if (shouldRenderModelInFirstPerson() && player) {
        auto flags = reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(player) + 0x1A0
        );

        uint32_t backup = *flags;
        *flags &= ~1u;

        if (g_playerRenderOriginal)
            g_playerRenderOriginal(player, renderParams, matrix);

        *flags = backup;
        return;
    }

    if (g_playerRenderOriginal)
        g_playerRenderOriginal(player, renderParams, matrix);
}

void applyFirstPersonOffset(float x, float y, float z) {
    if (!m_cameraPtr)
        return;

    auto base = reinterpret_cast<uintptr_t>(m_cameraPtr) + 0x10;
    float* pos = reinterpret_cast<float*>(base);

    if (!m_positionModified) {
        m_originalPos[0] = pos[0];
        m_originalPos[1] = pos[1];
        m_originalPos[2] = pos[2];
        m_positionModified = true;
    }

    pos[0] = m_originalPos[0] + x;
    pos[1] = m_originalPos[1] + y;
    pos[2] = m_originalPos[2] + z;
}

void restoreOriginalPosition() {
    if (!m_cameraPtr || !m_positionModified)
        return;

    auto pos = reinterpret_cast<float*>(
        reinterpret_cast<uintptr_t>(m_cameraPtr) + 0x10
    );

    pos[0] = m_originalPos[0];
    pos[1] = m_originalPos[1];
    pos[2] = m_originalPos[2];

    m_positionModified = false;
}

bool isFirstPerson() {
    if (!m_cameraPtr)
        return false;

    int mode = *reinterpret_cast<int*>(
        reinterpret_cast<uintptr_t>(m_cameraPtr) + 0x34
    );

    return mode == 0;
}

void onCameraRender() {
    if (isFirstPerson())
        applyFirstPersonOffset(
            m_cameraOffset.x,
            m_cameraOffset.y,
            m_cameraOffset.z
        );
}

bool shouldRenderModelInFirstPerson() {
    return m_enabled && isFirstPerson();
}

void RegisterCameraHooks() {
    HOOK("48 ?? ?? ?? 21 ?? ?? 91 ?? ?? 00 ?? ?? 91 ?? ?? 40 ?? ?? 00 00 ?? ?? 00 94", (void*)hookedCameraRender, (void**)&g_cameraRenderOriginal);
    HOOK("FD 7B ?? A9 F4 4F ?? A9 FD 03 00 91 F3 03 00 ?? ?? ?? 00 ?? ?? ?? 00 94", (void*)hookedCameraIsFirstPerson, (void**)&g_cameraIsFirstPersonOriginal);
}

void RegisterPlayerRendererHooks() {
    HOOK("FF 03 04 D1 FD 7B 0C A9 FA 67 0D A9 F8 5F 0E A9 F6 57 0F A9 F4 4F 10 A9", (void*)hookedPlayerRender, (void**)&g_playerRenderOriginal);
}

void RegisterOptionsFovHooks() {
    HOOK("FD 7B ?? A9 F4 4F ?? A9 FD 03 00 91 08 ?? 40 ?? ?? ?? 00 ?? ?? 00 00 ?? ?? ?? 00 94",(void*)hookedOptionsGetFov, (void**)&g_optionsGetFov);
}