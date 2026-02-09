#include "util/Hooks.hpp"

bool VHOOK(
    const char* typeinfoName,
    int vtableIndex,
    void* hook,
    void** orig
) {
    uintptr_t libBase = 0;
    std::ifstream maps("/proc/self/maps");
    std::string line;

    while (std::getline(maps, line)) {
        if (line.find("libminecraftpe.so") != std::string::npos &&
            line.find("r-xp") != std::string::npos) {
            sscanf(line.c_str(), "%lx-%*lx", &libBase);
            break;
        }
    }

    if (!libBase) {
        LOGE("VHOOK: !libBase");
        return false;
    }

    size_t nameLen = strlen(typeinfoName);
    uintptr_t typeinfoNameAddr = 0;

    std::ifstream maps2("/proc/self/maps");
    while (std::getline(maps2, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos &&
            line.find("r-xp") == std::string::npos) continue;

        uintptr_t start, end;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;

        for (uintptr_t a = start; a < end - nameLen; a++) {
            if (!memcmp((void*)a, typeinfoName, nameLen)) {
                typeinfoNameAddr = a;
                break;
            }
        }
        if (typeinfoNameAddr) break;
    }

    if (!typeinfoNameAddr) {
        LOGE("VHOOK: !typeinfoNameAddr");
        return false;
    }

    uintptr_t typeinfoAddr = 0;
    std::ifstream maps3("/proc/self/maps");
    while (std::getline(maps3, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos) continue;

        uintptr_t start, end;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;

        for (uintptr_t a = start; a < end; a += sizeof(void*)) {
            if (*(uintptr_t*)a == typeinfoNameAddr) {
                typeinfoAddr = a - sizeof(void*);
                break;
            }
        }
        if (typeinfoAddr) break;
    }

    if (!typeinfoAddr) {
        LOGE("VHOOK: !typeinfoAddr");
        return false;
    }

    uintptr_t vtableAddr = 0;
    std::ifstream maps4("/proc/self/maps");
    while (std::getline(maps4, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos) continue;

        uintptr_t start, end;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;

        for (uintptr_t a = start; a < end; a += sizeof(void*)) {
            if (*(uintptr_t*)a == typeinfoAddr) {
                vtableAddr = a + sizeof(void*);
                break;
            }
        }
        if (vtableAddr) break;
    }

    if (!vtableAddr) {
        LOGE("VHOOK: !vtableAddr");
        return false;
    }

    uintptr_t* slot = (uintptr_t*)(vtableAddr + vtableIndex * sizeof(void*));
    *orig = (void*)(*slot);

    uintptr_t page = (uintptr_t)slot & ~4095UL;
    mprotect((void*)page, 4096, PROT_READ | PROT_WRITE);
    *slot = (uintptr_t)hook;
    mprotect((void*)page, 4096, PROT_READ);

    LOGI("VHOOK: succes!");
    return true;
}

bool HOOK(
    const char* sig,
    void* hook,
    void** orig
) {
    uintptr_t addr = pl::signature::pl_resolve_signature(sig, "libminecraftpe.so");
    if (!addr) {
        LOGE("HOOK failed: signature not found");
        return false;
    }

    int ret = pl::hook::pl_hook((pl::hook::FuncPtr)addr, (pl::hook::FuncPtr)hook, (pl::hook::FuncPtr*)&orig, pl::hook::PriorityHighest);
    if (ret != 0) {
        LOGE("HOOK failed: {}", ret);
        return false;
    }

    LOGI("HOOK: success!");
    return true;
}