#include "HookManager.h"
#include "KittyMemory.h"
#include "Logger.h"
#include <cstring>
#include <sys/mman.h>

namespace FirstPersonModel {

// HookManager implementation
HookManager& HookManager::getInstance() {
    static HookManager instance;
    return instance;
}

HookManager::~HookManager() {
    unhookAll();
}

bool HookManager::initialize() {
    // Initialize hooking system
    if (!KittyMemory::init()) {
        LOGE("Failed to initialize KittyMemory in HookManager");
        return false;
    }
    
    m_initialized = true;
    return true;
}

bool HookManager::hookFunction(void* target, void* replacement, void** original) {
    if (!target || !replacement) {
        LOGE("Invalid parameters for hook");
        return false;
    }
    
    // Check if already hooked
    if (isHooked(target)) {
        LOGD("Function already hooked at %p", target);
        return false;
    }
    
    // Create hook info
    HookInfo info;
    info.target = target;
    info.replacement = replacement;
    info.isVirtualHook = false;
    info.vtableEntry = nullptr;
    info.originalVirtualFunction = nullptr;
    
    // Save original bytes (first 16 bytes for ARM64 - room for multiple instructions)
    const size_t BYTES_TO_SAVE = 16;
    info.originalBytes.resize(BYTES_TO_SAVE);
    if (!KittyMemory::readMemory(target, info.originalBytes.data(), BYTES_TO_SAVE)) {
        LOGE("Failed to read original bytes at %p", target);
        return false;
    }
    
    // Create trampoline (for calling original)
    if (original) {
        if (!createTrampoline(target, replacement, &info.original)) {
            LOGE("Failed to create trampoline for %p", target);
            return false;
        }
        *original = info.original;
    }
    
    // Write hook - branch to replacement
    // For ARM64, we use a B instruction if possible
    int64_t offset = reinterpret_cast<int64_t>(replacement) - reinterpret_cast<int64_t>(target);
    
    // Align to page for mprotect
    uintptr_t pageStart = reinterpret_cast<uintptr_t>(target) & ~(getpagesize() - 1);
    size_t pageLen = 16 + (reinterpret_cast<uintptr_t>(target) - pageStart);
    
    if (mprotect(reinterpret_cast<void*>(pageStart), pageLen, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        LOGE("Failed to change memory protection for %p", target);
        return false;
    }
    
    if (offset >= -(1LL << 25) && offset < (1LL << 25)) {
        // Can use direct B instruction
        uint32_t branchInsn = 0x14000000 | ((offset >> 2) & 0x03FFFFFF);
        *static_cast<uint32_t*>(target) = branchInsn;
        
        // NOP the rest
        const uint32_t nop = 0xD503201F;
        for (size_t i = 4; i < BYTES_TO_SAVE; i += 4) {
            *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(target) + i) = nop;
        }
    } else {
        // Need absolute jump - more complex
        // For now, we don't support this in the simple implementation
        LOGE("Target too far for relative branch at %p", target);
        
        // Restore original bytes
        restoreOriginalBytes(target, info.originalBytes);
        return false;
    }
    
    m_hooks.push_back(info);
    LOGI("Hook installed at %p -> %p", target, replacement);
    return true;
}

bool HookManager::hookVirtualFunction(void* object, int vtableIndex, void* replacement, void** original) {
    if (!object) {
        LOGE("Invalid object for virtual hook");
        return false;
    }
    
    // Get vtable
    void** vtable = *static_cast<void***>(object);
    if (!vtable) {
        LOGE("Failed to get vtable from object");
        return false;
    }
    
    // Get function address
    void** vtableEntry = &vtable[vtableIndex];
    void* target = *vtableEntry;
    
    if (!target) {
        LOGE("Virtual function at index %d is null", vtableIndex);
        return false;
    }
    
    // Save original
    if (original) {
        *original = target;
    }
    
    // Make vtable entry writable
    uintptr_t pageStart = reinterpret_cast<uintptr_t>(vtableEntry) & ~(getpagesize() - 1);
    size_t pageLen = sizeof(void*) + (reinterpret_cast<uintptr_t>(vtableEntry) - pageStart);
    
    if (mprotect(reinterpret_cast<void*>(pageStart), pageLen, PROT_READ | PROT_WRITE) != 0) {
        LOGE("Failed to make vtable writable");
        return false;
    }
    
    // Replace entry
    *vtableEntry = replacement;
    
    // Create hook info
    HookInfo info;
    info.target = target;
    info.replacement = replacement;
    info.isVirtualHook = true;
    info.vtableEntry = vtableEntry;
    info.originalVirtualFunction = target;
    info.original = target; // For virtual hooks, original is the old function
    
    m_hooks.push_back(info);
    
    LOGI("Virtual hook installed at index %d (%p -> %p)", vtableIndex, target, replacement);
    return true;
}

bool HookManager::hookPLT(void* pltEntry, void* replacement, void** original) {
    // PLT (Procedure Linkage Table) hooks are platform-specific
    // On ARM64, PLT entries typically jump to GOT (Global Offset Table)
    
    LOGE("PLT hooking not implemented");
    return false;
}

bool HookManager::unhookFunction(void* target) {
    for (auto it = m_hooks.begin(); it != m_hooks.end(); ++it) {
        if (it->target == target) {
            if (it->isVirtualHook && it->vtableEntry) {
                // Restore vtable entry
                *it->vtableEntry = it->originalVirtualFunction;
            } else {
                // Restore original bytes
                restoreOriginalBytes(target, it->originalBytes);
            }
            
            m_hooks.erase(it);
            LOGI("Hook removed from %p", target);
            return true;
        }
    }
    
    LOGD("Hook not found at %p", target);
    return false;
}

void HookManager::unhookAll() {
    for (const auto& hook : m_hooks) {
        if (hook.isVirtualHook && hook.vtableEntry) {
            *hook.vtableEntry = hook.originalVirtualFunction;
        } else {
            restoreOriginalBytes(hook.target, hook.originalBytes);
        }
    }
    
    m_hooks.clear();
    LOGI("All hooks removed");
}

bool HookManager::isHooked(void* target) const {
    for (const auto& hook : m_hooks) {
        if (hook.target == target) {
            return true;
        }
    }
    return false;
}

void** HookManager::getVTable(void* object) const {
    if (!object) {
        return nullptr;
    }
    return *static_cast<void***>(object);
}

void* HookManager::getVirtualFunction(void* object, int index) const {
    void** vtable = getVTable(object);
    if (!vtable) {
        return nullptr;
    }
    return vtable[index];
}

bool HookManager::setVirtualFunction(void* object, int index, void* function) {
    void** vtable = getVTable(object);
    if (!vtable) {
        return false;
    }
    
    // Make writable
    void** entry = &vtable[index];
    uintptr_t pageStart = reinterpret_cast<uintptr_t>(entry) & ~(getpagesize() - 1);
    size_t pageLen = sizeof(void*) + (reinterpret_cast<uintptr_t>(entry) - pageStart);
    
    if (mprotect(reinterpret_cast<void*>(pageStart), pageLen, PROT_READ | PROT_WRITE) != 0) {
        return false;
    }
    
    *entry = function;
    return true;
}

bool HookManager::createTrampoline(void* target, void* replacement, void** trampoline) {
    // Allocate executable memory for trampoline
    // This is simplified - real implementation needs proper memory management
    
    size_t pageSize = getpagesize();
    void* mem = mmap(nullptr, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (mem == MAP_FAILED) {
        LOGE("Failed to allocate memory for trampoline");
        return false;
    }
    
    // Copy original bytes (16 bytes)
    memcpy(mem, target, 16);
    
    // Add jump back to original function after the copied bytes
    void* returnAddr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(target) + 16);
    int64_t offset = reinterpret_cast<int64_t>(returnAddr) - 
                     (reinterpret_cast<int64_t>(mem) + 20); // 16 bytes + 4 for branch
    
    if (offset >= -(1LL << 25) && offset < (1LL << 25)) {
        uint32_t branchInsn = 0x14000000 | ((offset >> 2) & 0x03FFFFFF);
        *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(mem) + 16) = branchInsn;
    } else {
        // Need more complex trampoline
        munmap(mem, pageSize);
        LOGE("Offset too large for trampoline");
        return false;
    }
    
    *trampoline = mem;
    return true;
}

bool HookManager::restoreOriginalBytes(void* target, const std::vector<uint8_t>& bytes) {
    if (!target || bytes.empty()) {
        return false;
    }
    
    uintptr_t pageStart = reinterpret_cast<uintptr_t>(target) & ~(getpagesize() - 1);
    size_t pageLen = bytes.size() + (reinterpret_cast<uintptr_t>(target) - pageStart);
    
    if (mprotect(reinterpret_cast<void*>(pageStart), pageLen, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }
    
    memcpy(target, bytes.data(), bytes.size());
    return true;
}

// ScopedHook implementation
ScopedHook::ScopedHook(void* target, void* replacement, void** original) 
    : m_target(target), m_active(false) {
    auto& manager = HookManager::getInstance();
    if (manager.hookFunction(target, replacement, original)) {
        m_active = true;
    }
}

ScopedHook::~ScopedHook() {
    if (m_active) {
        auto& manager = HookManager::getInstance();
        manager.unhookFunction(m_target);
    }
}

void ScopedHook::release() {
    m_active = false;
}

} // namespace FirstPersonModel
