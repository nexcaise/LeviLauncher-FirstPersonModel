#pragma once

#include "ARMPatch.h"
#include "KittyMemory.h"
#include "KittyScanner.h"
#include <functional>

namespace FirstPersonModel {

// High-level hook manager that combines all hooking methods
class HookManager {
public:
    static HookManager& getInstance();
    
    bool initialize();
    void shutdown();
    
    // Hook installation
    bool hookFunction(void* target, void* replacement, void** original);
    bool hookVirtualFunction(void* object, int vtableIndex, void* replacement, void** original);
    bool hookPLT(void* pltEntry, void* replacement, void** original);
    
    // Hook removal
    bool unhookFunction(void* target);
    void unhookAll();
    
    // Hook status
    bool isHooked(void* target) const;
    bool isInitialized() const { return m_initialized; }
    
    // Hook installation with lambda (convenience)
    template<typename Func, typename Replacement>
    bool hook(void* target, Replacement&& replacement, Func** original) {
        return hookFunction(target, reinterpret_cast<void*>(replacement), reinterpret_cast<void**>(original));
    }
    
    // VTable hooking helpers
    void** getVTable(void* object) const;
    void* getVirtualFunction(void* object, int index) const;
    bool setVirtualFunction(void* object, int index, void* function);
    
private:
    HookManager() = default;
    ~HookManager();
    
    HookManager(const HookManager&) = delete;
    HookManager& operator=(const HookManager&) = delete;
    
    bool m_initialized = false;
    
    struct HookInfo {
        void* target;
        void* replacement;
        void* original;
        std::vector<uint8_t> originalBytes;
        bool isVirtualHook;
        void** vtableEntry;
        void* originalVirtualFunction;
    };
    
    std::vector<HookInfo> m_hooks;
    
    bool createTrampoline(void* target, void* replacement, void** trampoline);
    bool restoreOriginalBytes(void* target, const std::vector<uint8_t>& bytes);
};

// RAII hook guard
class ScopedHook {
public:
    ScopedHook(void* target, void* replacement, void** original);
    ~ScopedHook();
    
    ScopedHook(const ScopedHook&) = delete;
    ScopedHook& operator=(const ScopedHook&) = delete;
    
    bool isActive() const { return m_active; }
    void release();
    
private:
    void* m_target;
    bool m_active;
};

} // namespace FirstPersonModel
