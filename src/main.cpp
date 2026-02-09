#include "features/Features.hpp"
#include "util/Logger.h"


extern "C" __attribute__((constructor))
void FirstPersonModel_Init() {
    LOGI("========================================");
    LOGI("FirstPersonModel Library Loaded");
    LOGI("Version: 1.0.0");
    LOGI("Platform: Android ARM64");
    LOGI("Target: LeviLaunchroid / Minecraft Bedrock");
    LOGI("========================================");
    
    // Create initialization thread
    RegisterPlayerRendererHooks();
    RegisterCameraHooks();
}

// Destructor function - runs when library is unloaded
/*extern "C" __attribute__((destructor))
void FirstPersonModel_Shutdown() {
    LOGI("FirstPersonModel shutting down...");
    
    auto& mod = FirstPersonModel::FirstPersonModelMod::getInstance();
    mod.shutdown();
    
    LOGI("FirstPersonModel shutdown complete");
}*/