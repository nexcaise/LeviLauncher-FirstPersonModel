#include "KittyMemory.h"
#include "Logger.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <link.h>
#include <cstring>
#include <fstream>
#include <sstream>

namespace FirstPersonModel {

bool KittyMemory::s_initialized = false;
pid_t KittyMemory::s_processID = 0;

bool KittyMemory::init() {
    if (s_initialized) {
        return true;
    }
    
    s_processID = getpid();
    s_initialized = true;
    
    LOGD("KittyMemory initialized for PID %d", s_processID);
    return true;
}

bool KittyMemory::isInitialized() {
    return s_initialized;
}

bool KittyMemory::readMemory(const void* address, void* buffer, size_t len) {
    if (!address || !buffer || len == 0) {
        return false;
    }
    
    memcpy(buffer, address, len);
    return true;
}

bool KittyMemory::writeMemory(void* address, const void* buffer, size_t len) {
    if (!address || !buffer || len == 0) {
        return false;
    }
    
    // Make memory writable
    if (!makeMemoryRWX(address, len)) {
        return false;
    }
    
    memcpy(address, buffer, len);
    return true;
}

bool KittyMemory::writeMemory(void* address, std::vector<uint8_t> bytes) {
    return writeMemory(address, bytes.data(), bytes.size());
}

bool KittyMemory::nopMemory(void* address, size_t len) {
    if (!address || len == 0) {
        return false;
    }
    
    std::vector<uint8_t> nops(len, 0xD503201F); // ARM64 NOP
    return writeMemory(address, nops.data(), len);
}

bool KittyMemory::setMemoryProtection(void* address, size_t len, int protection) {
    if (!address || len == 0) {
        return false;
    }
    
    // Align address to page boundary
    uintptr_t addr = reinterpret_cast<uintptr_t>(address);
    uintptr_t pageStart = addr & ~(getpagesize() - 1);
    size_t pageLen = len + (addr - pageStart);
    
    int prot = 0;
    if (protection & MEMORY_PROTECTION_READ) prot |= PROT_READ;
    if (protection & MEMORY_PROTECTION_WRITE) prot |= PROT_WRITE;
    if (protection & MEMORY_PROTECTION_EXEC) prot |= PROT_EXEC;
    
    return mprotect(reinterpret_cast<void*>(pageStart), pageLen, prot) == 0;
}

bool KittyMemory::makeMemoryRWX(void* address, size_t len) {
    return setMemoryProtection(address, len, MEMORY_PROTECTION_READ_WRITE_EXEC);
}

bool KittyMemory::getMemoryInfo(void* address, void** base, size_t* size, int* protection) {
    if (!address) {
        return false;
    }
    
    // Parse /proc/self/maps
    std::ifstream maps("/proc/self/maps");
    if (!maps.is_open()) {
        return false;
    }
    
    std::string line;
    uintptr_t addr = reinterpret_cast<uintptr_t>(address);
    
    while (std::getline(maps, line)) {
        uintptr_t start, end;
        char perms[5];
        
        if (sscanf(line.c_str(), "%lx-%lx %4s", &start, &end, perms) == 3) {
            if (addr >= start && addr < end) {
                if (base) *base = reinterpret_cast<void*>(start);
                if (size) *size = end - start;
                if (protection) {
                    *protection = 0;
                    if (perms[0] == 'r') *protection |= MEMORY_PROTECTION_READ;
                    if (perms[1] == 'w') *protection |= MEMORY_PROTECTION_WRITE;
                    if (perms[2] == 'x') *protection |= MEMORY_PROTECTION_EXEC;
                }
                return true;
            }
        }
    }
    
    return false;
}

bool KittyMemory::isExecutable(void* address) {
    int protection = 0;
    if (getMemoryInfo(address, nullptr, nullptr, &protection)) {
        return protection & MEMORY_PROTECTION_EXEC;
    }
    return false;
}

bool KittyMemory::isReadable(void* address) {
    int protection = 0;
    if (getMemoryInfo(address, nullptr, nullptr, &protection)) {
        return protection & MEMORY_PROTECTION_READ;
    }
    return false;
}

bool KittyMemory::isWritable(void* address) {
    int protection = 0;
    if (getMemoryInfo(address, nullptr, nullptr, &protection)) {
        return protection & MEMORY_PROTECTION_WRITE;
    }
    return false;
}

void* KittyMemory::getLibraryBase(const char* libraryName) {
    if (!libraryName) {
        return nullptr;
    }
    
    Dl_info info;
    void* handle = dlopen(libraryName, RTLD_NOW);
    
    if (!handle) {
        // Try to find it in loaded libraries
        std::ifstream maps("/proc/self/maps");
        if (!maps.is_open()) {
            return nullptr;
        }
        
        std::string line;
        while (std::getline(maps, line)) {
            if (line.find(libraryName) != std::string::npos) {
                uintptr_t start;
                if (sscanf(line.c_str(), "%lx-", &start) == 1) {
                    return reinterpret_cast<void*>(start);
                }
            }
        }
        return nullptr;
    }
    
    // Get any symbol to find base address
    void* sym = dlsym(handle, "JNI_OnLoad");
    if (!sym) {
        sym = dlsym(handle, "_init");
    }
    
    if (sym && dladdr(sym, &info)) {
        dlclose(handle);
        return const_cast<void*>(info.dli_fbase);
    }
    
    dlclose(handle);
    return nullptr;
}

size_t KittyMemory::getLibrarySize(const char* libraryName) {
    void* base = getLibraryBase(libraryName);
    if (!base) {
        return 0;
    }
    
    std::ifstream maps("/proc/self/maps");
    if (!maps.is_open()) {
        return 0;
    }
    
    std::string line;
    size_t totalSize = 0;
    uintptr_t baseAddr = reinterpret_cast<uintptr_t>(base);
    
    while (std::getline(maps, line)) {
        if (line.find(libraryName) != std::string::npos) {
            uintptr_t start, end;
            if (sscanf(line.c_str(), "%lx-%lx", &start, &end) == 2) {
                if (start >= baseAddr) {
                    totalSize += (end - start);
                }
            }
        }
    }
    
    return totalSize;
}

bool KittyMemory::isLibraryLoaded(const char* libraryName) {
    return getLibraryBase(libraryName) != nullptr;
}

void* KittyMemory::getSymbol(void* base, const char* symbol) {
    if (!base || !symbol) {
        return nullptr;
    }
    
    Dl_info info;
    if (dladdr(base, &info) && info.dli_fname) {
        void* handle = dlopen(info.dli_fname, RTLD_NOW);
        if (handle) {
            void* sym = dlsym(handle, symbol);
            dlclose(handle);
            return sym;
        }
    }
    
    return nullptr;
}

pid_t KittyMemory::getProcessID() {
    if (s_processID == 0) {
        s_processID = getpid();
    }
    return s_processID;
}

std::string KittyMemory::getProcessName() {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/cmdline", getProcessID());
    
    std::ifstream file(path);
    if (file.is_open()) {
        std::string name;
        std::getline(file, name);
        return name;
    }
    
    return "";
}

// MemoryPatch implementation
MemoryPatch::MemoryPatch() = default;

MemoryPatch::MemoryPatch(void* address, const std::vector<uint8_t>& patchBytes) 
    : m_address(address), m_patchBytes(patchBytes) {
    // Save original bytes
    if (address && !patchBytes.empty()) {
        m_originalBytes.resize(patchBytes.size());
        KittyMemory::readMemory(address, m_originalBytes.data(), patchBytes.size());
    }
}

MemoryPatch::~MemoryPatch() {
    if (m_applied) {
        restore();
    }
}

bool MemoryPatch::apply() {
    if (m_applied || !m_address || m_patchBytes.empty()) {
        return false;
    }
    
    if (KittyMemory::writeMemory(m_address, m_patchBytes.data(), m_patchBytes.size())) {
        m_applied = true;
        return true;
    }
    
    return false;
}

bool MemoryPatch::restore() {
    if (!m_applied || !m_address || m_originalBytes.empty()) {
        return false;
    }
    
    if (KittyMemory::writeMemory(m_address, m_originalBytes.data(), m_originalBytes.size())) {
        m_applied = false;
        return true;
    }
    
    return false;
}

void MemoryPatch::setAddress(void* address) {
    m_address = address;
}

void MemoryPatch::setPatchBytes(const std::vector<uint8_t>& bytes) {
    m_patchBytes = bytes;
    if (m_address && !bytes.empty()) {
        m_originalBytes.resize(bytes.size());
        KittyMemory::readMemory(m_address, m_originalBytes.data(), bytes.size());
    }
}

void MemoryPatch::setOriginalBytes(const std::vector<uint8_t>& bytes) {
    m_originalBytes = bytes;
}

} // namespace FirstPersonModel
