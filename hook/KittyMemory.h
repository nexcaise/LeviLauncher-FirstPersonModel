#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace FirstPersonModel {

// Memory patching utilities
class KittyMemory {
public:
    static bool init();
    static bool isInitialized();
    
    // Memory operations
    static bool readMemory(const void* address, void* buffer, size_t len);
    static bool writeMemory(void* address, const void* buffer, size_t len);
    static bool writeMemory(void* address, std::vector<uint8_t> bytes);
    static bool nopMemory(void* address, size_t len);
    
    // Memory protection
    static bool setMemoryProtection(void* address, size_t len, int protection);
    static bool makeMemoryRWX(void* address, size_t len);
    
    // Memory information
    static bool getMemoryInfo(void* address, void** base, size_t* size, int* protection);
    static bool isExecutable(void* address);
    static bool isReadable(void* address);
    static bool isWritable(void* address);
    
    // Library management
    static void* getLibraryBase(const char* libraryName);
    static size_t getLibrarySize(const char* libraryName);
    static bool isLibraryLoaded(const char* libraryName);
    static void* getSymbol(void* base, const char* symbol);
    
    // Process information
    static pid_t getProcessID();
    static std::string getProcessName();
    
private:
    static bool s_initialized;
    static pid_t s_processID;
};

// Memory patch class
class MemoryPatch {
public:
    MemoryPatch();
    MemoryPatch(void* address, const std::vector<uint8_t>& patchBytes);
    ~MemoryPatch();
    
    bool apply();
    bool restore();
    bool isApplied() const { return m_applied; }
    
    void setAddress(void* address);
    void setPatchBytes(const std::vector<uint8_t>& bytes);
    void setOriginalBytes(const std::vector<uint8_t>& bytes);
    
private:
    void* m_address = nullptr;
    std::vector<uint8_t> m_patchBytes;
    std::vector<uint8_t> m_originalBytes;
    bool m_applied = false;
};

// Memory protection constants (match Android protection flags)
constexpr int MEMORY_PROTECTION_NONE = 0;
constexpr int MEMORY_PROTECTION_READ = 1;
constexpr int MEMORY_PROTECTION_WRITE = 2;
constexpr int MEMORY_PROTECTION_EXEC = 4;
constexpr int MEMORY_PROTECTION_READ_WRITE = MEMORY_PROTECTION_READ | MEMORY_PROTECTION_WRITE;
constexpr int MEMORY_PROTECTION_READ_WRITE_EXEC = MEMORY_PROTECTION_READ | MEMORY_PROTECTION_WRITE | MEMORY_PROTECTION_EXEC;

} // namespace FirstPersonModel
