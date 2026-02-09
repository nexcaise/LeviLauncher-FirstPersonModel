#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace FirstPersonModel {

// ARM64 memory hooking utilities
class ARMPatch {
public:
    static bool init();
    static bool isInitialized();
    
    // Memory access
    static bool writeMemory(void* address, const void* data, size_t size);
    static bool readMemory(void* address, void* buffer, size_t size);
    static bool nopMemory(void* address, size_t size);
    static bool writeBytes(void* address, const std::vector<uint8_t>& bytes);
    
    // Hooking
    static bool hookFunction(void* target, void* replacement, void** original);
    static bool unhookFunction(void* target);
    
    // Memory protection
    static bool setMemoryProtection(void* address, size_t size, int protection);
    static bool makeMemoryWritable(void* address, size_t size);
    static bool makeMemoryExecutable(void* address, size_t size);
    
    // Address calculations
    static void* getAddressFromOffset(void* base, int offset);
    static int64_t calculateBranchOffset(void* from, void* to);
    
    // ARM64 specific
    static bool writeBranch(void* from, void* to);
    static bool writeAbsoluteJump(void* from, void* to);
    
private:
    static bool s_initialized;
};

// Inline hook structure
struct InlineHook {
    void* targetAddress;
    void* replacementFunction;
    void* trampolineFunction;
    std::vector<uint8_t> originalBytes;
    bool isActive;
};

class HookManager {
public:
    static HookManager& getInstance();
    
    bool installHook(void* target, void* replacement, void** original);
    bool removeHook(void* target);
    void removeAllHooks();
    
    bool isHooked(void* target) const;
    
private:
    HookManager() = default;
    ~HookManager();
    
    std::vector<InlineHook> m_hooks;
};

// Memory protection constants
constexpr int MEMORY_PROTECTION_READ = 1;
constexpr int MEMORY_PROTECTION_WRITE = 2;
constexpr int MEMORY_PROTECTION_EXEC = 4;
constexpr int MEMORY_PROTECTION_READ_WRITE = MEMORY_PROTECTION_READ | MEMORY_PROTECTION_WRITE;
constexpr int MEMORY_PROTECTION_READ_EXEC = MEMORY_PROTECTION_READ | MEMORY_PROTECTION_EXEC;
constexpr int MEMORY_PROTECTION_ALL = MEMORY_PROTECTION_READ | MEMORY_PROTECTION_WRITE | MEMORY_PROTECTION_EXEC;

} // namespace FirstPersonModel
