#include "ARMPatch.h"
#include "Logger.h"
#include <sys/mman.h>
#include <cstring>
#include <dlfcn.h>

namespace FirstPersonModel {

bool ARMPatch::s_initialized = false;

bool ARMPatch::init() {
    if (s_initialized) {
        return true;
    }
    
    // Nothing special to initialize for basic ARM patching
    s_initialized = true;
    return true;
}

bool ARMPatch::isInitialized() {
    return s_initialized;
}

bool ARMPatch::writeMemory(void* address, const void* data, size_t size) {
    if (!address || !data || size == 0) {
        return false;
    }
    
    // Make memory writable
    if (!makeMemoryWritable(address, size)) {
        LOGE("Failed to make memory writable at %p", address);
        return false;
    }
    
    // Write data
    memcpy(address, data, size);
    
    // Restore original protection
    // Note: For executable code, we typically want to keep it executable
    
    return true;
}

bool ARMPatch::readMemory(void* address, void* buffer, size_t size) {
    if (!address || !buffer || size == 0) {
        return false;
    }
    
    memcpy(buffer, address, size);
    return true;
}

bool ARMPatch::nopMemory(void* address, size_t size) {
    if (!address || size == 0) {
        return false;
    }
    
    // ARM64 NOP instruction: D503201F
    const uint32_t nop = 0xD503201F;
    
    // Calculate number of NOPs needed
    size_t numNops = size / 4;
    if (size % 4 != 0) {
        // Handle partial instruction
        LOGE("NOP size must be multiple of 4 bytes");
        return false;
    }
    
    // Make memory writable
    if (!makeMemoryWritable(address, size)) {
        return false;
    }
    
    // Write NOPs
    uint32_t* ptr = static_cast<uint32_t*>(address);
    for (size_t i = 0; i < numNops; i++) {
        ptr[i] = nop;
    }
    
    return true;
}

bool ARMPatch::writeBytes(void* address, const std::vector<uint8_t>& bytes) {
    return writeMemory(address, bytes.data(), bytes.size());
}

bool ARMPatch::hookFunction(void* target, void* replacement, void** original) {
    if (!target || !replacement) {
        return false;
    }
    
    // This is a simplified inline hook implementation
    // For production use, consider using Dobby, Substrate, or similar
    
    // Save original function start (for calling original)
    if (original) {
        // Allocate trampoline
        // In real implementation, we'd allocate executable memory
        // and copy the original instructions there
        *original = target;
    }
    
    // Write branch to replacement function
    // For ARM64: B instruction is 26-bit signed offset
    int64_t offset = reinterpret_cast<int64_t>(replacement) - reinterpret_cast<int64_t>(target);
    
    // Check if offset fits in 26 bits
    if (offset < -(1LL << 25) || offset >= (1LL << 25)) {
        // Need absolute jump using BR
        // This requires a register and is more complex
        LOGE("Offset too large for B instruction, need indirect jump");
        return false;
    }
    
    // Create branch instruction
    // B: 0001 01xx ... offset is shifted right by 2
    uint32_t branchInsn = 0x14000000 | ((offset >> 2) & 0x03FFFFFF);
    
    if (!makeMemoryWritable(target, 4)) {
        return false;
    }
    
    *static_cast<uint32_t*>(target) = branchInsn;
    
    return true;
}

bool ARMPatch::unhookFunction(void* target) {
    if (!target) {
        return false;
    }
    
    // To unhook, we need to restore the original bytes
    // This requires storing them somewhere during hook
    
    LOGE("Unhook not fully implemented - requires stored original bytes");
    return false;
}

bool ARMPatch::setMemoryProtection(void* address, size_t size, int protection) {
    if (!address || size == 0) {
        return false;
    }
    
    // Align to page boundary
    uintptr_t addr = reinterpret_cast<uintptr_t>(address);
    uintptr_t pageStart = addr & ~(getpagesize() - 1);
    size_t pageSize = size + (addr - pageStart);
    
    int prot = 0;
    if (protection & MEMORY_PROTECTION_READ) prot |= PROT_READ;
    if (protection & MEMORY_PROTECTION_WRITE) prot |= PROT_WRITE;
    if (protection & MEMORY_PROTECTION_EXEC) prot |= PROT_EXEC;
    
    return mprotect(reinterpret_cast<void*>(pageStart), pageSize, prot) == 0;
}

bool ARMPatch::makeMemoryWritable(void* address, size_t size) {
    return setMemoryProtection(address, size, MEMORY_PROTECTION_READ_WRITE_EXEC);
}

bool ARMPatch::makeMemoryExecutable(void* address, size_t size) {
    return setMemoryProtection(address, size, MEMORY_PROTECTION_READ_EXEC);
}

void* ARMPatch::getAddressFromOffset(void* base, int offset) {
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(base) + offset);
}

int64_t ARMPatch::calculateBranchOffset(void* from, void* to) {
    return reinterpret_cast<int64_t>(to) - reinterpret_cast<int64_t>(from);
}

bool ARMPatch::writeBranch(void* from, void* to) {
    int64_t offset = calculateBranchOffset(from, to);
    
    // Check if it fits in 26-bit signed offset
    if (offset < -(1LL << 25) || offset >= (1LL << 25)) {
        return false;
    }
    
    uint32_t branchInsn = 0x14000000 | ((offset >> 2) & 0x03FFFFFF);
    return writeMemory(from, &branchInsn, 4);
}

bool ARMPatch::writeAbsoluteJump(void* from, void* to) {
    // ARM64 absolute jump requires loading address into register then BR
    // This is more complex and typically requires a trampoline
    
    // Simplified version - doesn't handle all cases
    // For real implementation, use a proper hooking library
    
    LOGE("Absolute jump not implemented - use proper hooking library");
    return false;
}

} // namespace FirstPersonModel
