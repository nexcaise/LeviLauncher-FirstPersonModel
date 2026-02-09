#include <jni.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cmath>
#include <string>
#include "pl/Gloss.h"

static float g_CameraOffsetX = 0.0f;
static float g_CameraOffsetY = -0.2f;
static float g_CameraOffsetZ = 0.5f;
static float g_ModelScale = 1.0f;

static void* (*CameraGetPosition)(void* camera, float* x, float* y, float* z) = nullptr;
static void* (*CameraSetPosition)(void* camera, float x, float y, float z) = nullptr;
static void* (*CameraGetRotation)(void* camera, float* pitch, float* yaw) = nullptr;
static void* (*CameraSetRotation)(void* camera, float pitch, float yaw) = nullptr;
static void* (*RenderPlayerModel)(void* player, float deltaTime) = nullptr;
static void* (*orig_CameraRender)(void* camera, float deltaTime) = nullptr;

static void* hook_CameraRender(void* camera, float deltaTime) {
    if (RenderPlayerModel) {
        float camX, camY, camZ;
        float pitch, yaw;
        
        if (CameraGetPosition) {
            CameraGetPosition(camera, &camX, &camY, &camZ);
            CameraSetPosition(camera, camX + g_CameraOffsetX, camY + g_CameraOffsetY, camZ + g_CameraOffsetZ);
        }
        
        void* player = nullptr;
        if (player) {
            RenderPlayerModel(player, deltaTime);
        }
        
        if (CameraGetPosition) {
            CameraSetPosition(camera, camX, camY, camZ);
        }
    }
    
    return orig_CameraRender ? orig_CameraRender(camera, deltaTime) : nullptr;
}

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
    
    RenderPlayerModel = (void* (*)(void*, float))dlsym(mcpe,
        "_ZN6Player11renderModelEf");
}

static void HookCamera() {
    void* mcpe = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!mcpe) return;
    
    void* cameraRender = dlsym(mcpe, "_ZN6Camera6renderEf");
    if (cameraRender) {
        GlossHook(cameraRender, (void*)hook_CameraRender, (void**)&orig_CameraRender);
    }
}

__attribute__((constructor))
void FirstPersonModel_Init() {
    GlossInit(true);
    InitCameraAPI();
    HookCamera();
}