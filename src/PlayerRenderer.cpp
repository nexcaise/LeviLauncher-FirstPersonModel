#include "Logger.h"
#include "FirstPersonModel.h"
#include "Offsets.h"
#include "hook/HookManager.h"
#include "Signatures.h"
#include <cstring>
#include <dlfcn.h>

namespace FirstPersonModel {

// Player render function signature
using PlayerRenderOriginal = void(*)(void* player, void* renderParams, void* matrix);

static void* g_playerRenderOriginal = nullptr;

void hookedPlayerRender(void* player, void* renderParams, void* matrix) {
    auto& mod = FirstPersonModelMod::getInstance();
    
    // Check if we should render in first person
    if (mod.shouldRenderModelInFirstPerson()) {
        // Force player model to render even in first person
        // This typically involves modifying flags or calling render with different parameters
        
        if (player) {
            auto& offsetManager = OffsetManager::getInstance();
            
            // Modify player render flags to allow first person rendering
            // This offset may need adjustment based on version
            size_t renderFlagsOffset = 0x1A0; // Example offset
            uint32_t* renderFlags = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(player) + renderFlagsOffset);
            
            // Save original flags
            uint32_t originalFlags = *renderFlags;
            
            // Clear first person hide flag (bit 0 usually controls visibility)
            *renderFlags &= ~0x1;
            
            // Call original render
            if (g_playerRenderOriginal) {
                reinterpret_cast<PlayerRenderOriginal>(g_playerRenderOriginal)(player, renderParams, matrix);
            }
            
            // Restore flags
            *renderFlags = originalFlags;
        }
    } else {
        // Normal render path
        if (g_playerRenderOriginal) {
            reinterpret_cast<PlayerRenderOriginal>(g_playerRenderOriginal)(player, renderParams, matrix);
        }
    }
    
    mod.onPlayerRender();
}

bool InitializePlayerRenderer() {
    LOGI("Initializing Player Renderer hooks");
    
    auto& offsetManager = OffsetManager::getInstance();
    void* playerRenderAddr = offsetManager.getPlayerRenderAddress();
    
    if (!playerRenderAddr) {
        LOGE("Player render function not found");
        return false;
    }
    
    auto& hookManager = HookManager::getInstance();
    if (!hookManager.hook(playerRenderAddr,
                          reinterpret_cast<void*>(hookedPlayerRender),
                          reinterpret_cast<void**>(&g_playerRenderOriginal))) {
        LOGE("Failed to hook player render function");
        return false;
    }
    
    LOGI("Player renderer initialized successfully");
    return true;
}

void ShutdownPlayerRenderer() {
    if (g_playerRenderOriginal) {
        auto& hookManager = HookManager::getInstance();
        hookManager.unhookFunction(g_playerRenderOriginal);
        g_playerRenderOriginal = nullptr;
    }
}

} // namespace FirstPersonModel
