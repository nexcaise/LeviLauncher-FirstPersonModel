#include "Logger.h"
#include "FirstPersonModel.h"
#include "hook/KittyMemory.h"
#include "hook/HookManager.h"
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>

// Main mod entry point
namespace FirstPersonModel {

// Thread function for initialization
void* modInitThread(void* arg) {
    LOGI("FirstPersonModel initialization thread started");
    
    // Wait for Minecraft to fully load
    // This is important to ensure all necessary libraries are loaded
    int attempts = 0;
    const char* minecraftLib = "libminecraftpe.so";
    
    while (!KittyMemory::isLibraryLoaded(minecraftLib) && attempts < 100) {
        usleep(100000); // 100ms
        attempts++;
    }
    
    if (!KittyMemory::isLibraryLoaded(minecraftLib)) {
        LOGE("Minecraft library not loaded after 10 seconds");
        return nullptr;
    }
    
    // Additional wait for game initialization
    usleep(500000); // 500ms
    
    LOGI("Minecraft library detected, initializing mod...");
    
    // Initialize memory system
    if (!KittyMemory::init()) {
        LOGE("Failed to initialize KittyMemory");
        return nullptr;
    }
    
    // Initialize hook manager
    auto& hookManager = HookManager::getInstance();
    if (!hookManager.initialize()) {
        LOGE("Failed to initialize HookManager");
        return nullptr;
    }
    
    // Initialize the mod
    auto& mod = FirstPersonModelMod::getInstance();
    if (!mod.initialize()) {
        LOGE("Failed to initialize FirstPersonModel");
        return nullptr;
    }
    
    LOGI("FirstPersonModel mod loaded successfully!");
    LOGI("Features:");
    LOGI("  - First Person Model: ENABLED");
    LOGI("  - Camera Offset: ENABLED");
    LOGI("  - Client-side only: YES");
    LOGI("  - No ImGui: YES");
    
    return nullptr;
}

} // namespace FirstPersonModel

// Constructor function - runs when library is loaded
extern "C" __attribute__((constructor))
void FirstPersonModel_Init() {
    LOGI("========================================");
    LOGI("FirstPersonModel Library Loaded");
    LOGI("Version: 1.0.0");
    LOGI("Platform: Android ARM64");
    LOGI("Target: LeviLaunchroid / Minecraft Bedrock");
    LOGI("========================================");
    
    // Create initialization thread
    pthread_t initThread;
    pthread_create(&initThread, nullptr, FirstPersonModel::modInitThread, nullptr);
    pthread_detach(initThread);
}

// Destructor function - runs when library is unloaded
extern "C" __attribute__((destructor))
void FirstPersonModel_Shutdown() {
    LOGI("FirstPersonModel shutting down...");
    
    auto& mod = FirstPersonModel::FirstPersonModelMod::getInstance();
    mod.shutdown();
    
    LOGI("FirstPersonModel shutdown complete");
}

// JNI_OnLoad - required for Android
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    (void)vm;
    (void)reserved;
    LOGI("FirstPersonModel JNI_OnLoad called");
    return JNI_VERSION_1_6;
}
