#include <unistd.h>
#include <dlfcn.h>
#include <cmath>
#include <string>

#include <cstdint>

#include "pl/Signature.h"
#include "pl/Hook.h"
#include "util/Logger.hpp"

#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/mman.h>

#include "pl/Gloss.h"

bool HOOK(
    const char* sig,
    void* hook,
    void** orig
) {
    uintptr_t addr = pl::signature::pl_resolve_signature(sig, "libminecraftpe.so");
    if (!addr) {
        return false;
    }

    int ret = pl::hook::pl_hook((pl::hook::FuncPtr)addr, (pl::hook::FuncPtr)hook, (pl::hook::FuncPtr*)&orig, pl::hook::PriorityHighest);
    if (ret != 0) {
        return false;
    }

    return true;
}

// Camera offset values
static float g_CameraOffsetX = 0.0f;
static float g_CameraOffsetY = -0.2f;
static float g_CameraOffsetZ = 0.5f;
static float g_ModelScale = 1.0f;

// Function signatures
//static void* (*CameraGetPosition)(void* camera, float* x, float* y, float* z) = nullptr;
//static void* (*CameraSetPosition)(void* camera, float x, float y, float z) = nullptr;
//static void* (*CameraGetRotation)(void* camera, float* pitch, float* yaw) = nullptr;
//static void* (*CameraSetRotation)(void* camera, float pitch, float yaw) = nullptr;
//static void* (*RenderPlayerModel)(void* player, float deltaTime) = nullptr;

void CameraSetPosition(void* camera, float x, float y, float z) {
    if (!camera) {
        return;
    }
    
    // Read current position
    size_t posOffset = 0x18;
    float* posX = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(camera) + posOffset);
    float* posY = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(camera) + posOffset + sizeof(float));
    float* posZ = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(camera) + posOffset + 2 * sizeof(float));
    
    *posX = x;
    *posY = y;
    *posZ = z;
}

static void* (*orig_CameraRender)(void* camera, float deltaTime) = nullptr;
static void* hook_CameraRender(void* camera, float deltaTime) {
    //if (g_FirstPersonModelEnabled && RenderPlayerModel) {
        // Get current camera position and rotation
        float camX, camY, camZ;
        float pitch, yaw;
        
        //if (CameraGetPosition) {
            //CameraGetPosition(camera, &camX, &camY, &camZ);
            // Adjust camera position for first person model view
            CameraSetPosition(camera, camX + g_CameraOffsetX, camY + g_CameraOffsetY, camZ + g_CameraOffsetZ);
        //}
        
        // Render player model
        /*void* player = nullptr; // Get local player instance
        if (player) {
            RenderPlayerModel(player, deltaTime);
        }*/
        
        // Restore original camera position
        //if (CameraGetPosition) {
            CameraSetPosition(camera, camX, camY, camZ);
        //}
    //}
    
    return orig_CameraRender ? orig_CameraRender(camera, deltaTime) : nullptr;
}

/*
static void InitCameraAPI() {
    void* mcpe = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!mcpe) return;
    
    CameraGetPosition = (void* (*)(void*, float*, float*, float*))dlsym(mcpe, 
        "_ZN6Camera11getPositionEPfS0_S0_");
    CameraSetPosition = (void* (*)(void*, float, float, float))dlsym(mcpe,
        "_ZN6Camera11setPositionEfff");
    CameraGetRotation = (void* (*)(void*, float*, float*))dlsym(mcpe,
        "_ZN6Camera11getRotationEPfS0_");
    CameraSetRotation = (void* (*)(void*, float, float))dlsym(mcpe,
        "_ZN6Camera11setRotationEff");
    
    // Player model render function
    RenderPlayerModel = (void* (*)(void*, float))dlsym(mcpe,
        "_ZN6Player11renderModelEf");
}
*/

static void HookCamera() {
    HOOK("48 ?? ?? ?? 21 ?? ?? 91 ?? ?? 00 ?? ?? 91 ?? ?? 40 ?? ?? 00 00 ?? ?? 00 94", (void*)hook_CameraRender, (void**)&orig_CameraRender);
}

__attribute__((constructor))
void FirstPersonModel_Init() {
    GlossInit(true);
    //InitCameraAPI();
    HookCamera();
}