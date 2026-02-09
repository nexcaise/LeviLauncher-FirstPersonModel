#include "KittyArm64.h"
#include "Logger.h"
#include <cstring>

namespace FirstPersonModel {

bool KittyArm64::decodeInstruction(uint32_t instruction, uint32_t& opcode, uint32_t& rd, uint32_t& rn, uint32_t& rm) {
    // Basic decoding - extract common fields
    // This is a simplified decoder
    
    // Get opcode (bits 22-27 for data processing, varies for other types)
    opcode = (instruction >> 22) & 0x3F;
    
    // Get register fields (varies by instruction type)
    // Rd = bits 0-4 (destination)
    rd = instruction & 0x1F;
    
    // Rn = bits 5-9 (first source)
    rn = (instruction >> 5) & 0x1F;
    
    // Rm = bits 16-20 (second source)
    rm = (instruction >> 16) & 0x1F;
    
    return true;
}

uint32_t KittyArm64::makeBranchInstruction(int64_t offset) {
    // B (unconditional branch): 0001 01xx xxxx xxxx xxxx xxxx xxxx xxxx
    // Offset is 26 bits, signed, shifted left by 2
    int32_t imm26 = static_cast<int32_t>(offset >> 2) & 0x03FFFFFF;
    return 0x14000000 | imm26;
}

uint32_t KittyArm64::makeAbsoluteBranch(void* target) {
    // For absolute branches, we typically use BR or BLR with register
    // This creates a sequence, but single instruction version:
    // Use B with PC-relative offset calculation
    return makeBranchInstruction(reinterpret_cast<int64_t>(target));
}

uint32_t KittyArm64::makeNOP() {
    // NOP in ARM64: D503201F
    return 0xD503201F;
}

uint32_t KittyArm64::makeRet() {
    // RET: D65F03C0
    return 0xD65F03C0;
}

uint32_t KittyArm64::makeMovRegister(uint32_t rd, uint32_t rn) {
    // MOV (register): ORR Rd, XZR, Rn
    // ORR (shifted register): 0100 1010 0000 xxxx xxxx xxxx xxx xxxxx
    return 0x2A0003E0 | (rn << 16) | rd;
}

uint32_t KittyArm64::makeMovImmediate(uint32_t rd, uint16_t imm, uint32_t shift) {
    // MOVZ: 1x01 0010 1xx xxxx xxxx xxxx xxxx xxxx
    // shift: 0, 16, 32, or 48 bits
    uint32_t hw = (shift / 16) & 0x3;
    return 0xD2800000 | (hw << 21) | (static_cast<uint32_t>(imm) << 5) | rd;
}

bool KittyArm64::isBranchInstruction(uint32_t instruction) {
    // B, BL, B.cond, CBZ, CBNZ, TBZ, TBNZ
    uint32_t op = (instruction >> 26) & 0x1F;
    return (op == 0x05) || // B, BL
           (op == 0x15) || // B.cond
           ((instruction & 0x7E000000) == 0x34000000) || // CBZ/CBNZ
           ((instruction & 0x7E000000) == 0x36000000);   // TBZ/TBNZ
}

bool KittyArm64::isBLInstruction(uint32_t instruction) {
    // BL: 1001 01xx xxxx xxxx xxxx xxxx xxxx xxxx
    return ((instruction >> 26) & 0x3F) == 0x25;
}

bool KittyArm64::isBInstruction(uint32_t instruction) {
    // B (unconditional): 0001 01xx xxxx xxxx xxxx xxxx xxxx xxxx
    return ((instruction >> 26) & 0x3F) == 0x05;
}

bool KittyArm64::isCBZInstruction(uint32_t instruction) {
    // CBZ/CBNZ: x011 010x xxxx xxxx xxxx xxxx xxxx xxxx
    return ((instruction >> 24) & 0x7F) == 0x34;
}

bool KittyArm64::isTBZInstruction(uint32_t instruction) {
    // TBZ/TBNZ: x011 011x xxxx xxxx xxxx xxxx xxxx xxxx
    return ((instruction >> 24) & 0x7F) == 0x36;
}

bool KittyArm64::isADRPInstruction(uint32_t instruction) {
    // ADRP: 1x00 1000 xxxx xxxx xxxx xxxx xxxx xxxx
    return ((instruction >> 24) & 0x9F) == 0x90;
}

bool KittyArm64::isADDInstruction(uint32_t instruction) {
    // ADD (immediate): 0x1 0001 x...
    // ADD (shifted register): 0x0 1011 x...
    uint32_t op = (instruction >> 24) & 0xFF;
    return (op & 0xFC) == 0x91 || // ADD immediate
           (op & 0xFE) == 0x0B;    // ADD register
}

bool KittyArm64::isLDRInstruction(uint32_t instruction) {
    // LDR (immediate): x1x1 1000 xxxx xxxx xxxx xxxx xxxx xxxx
    // LDR (literal): x1x1 1000 xxxx xxxx xxxx xxxx xxxx xxxx
    return ((instruction >> 24) & 0xFE) == 0x58 ||
           ((instruction >> 24) & 0xFE) == 0x98;
}

int64_t KittyArm64::getBranchOffset(uint32_t instruction) {
    if (isBInstruction(instruction) || isBLInstruction(instruction)) {
        // 26-bit signed offset, shifted left by 2
        int32_t imm26 = instruction & 0x03FFFFFF;
        // Sign extend
        if (imm26 & 0x02000000) {
            imm26 |= 0xFC000000;
        }
        return static_cast<int64_t>(imm26) << 2;
    }
    return 0;
}

void* KittyArm64::calculateADRPAddress(void* base, uint32_t adrp, uint32_t add) {
    if (!isADRPInstruction(adrp)) {
        return nullptr;
    }
    
    // Decode ADRP
    // immhi = bits 23:5
    // immlo = bits 30:29
    uint64_t immhi = (adrp >> 5) & 0x7FFFF;
    uint64_t immlo = (adrp >> 29) & 0x3;
    
    // Sign extend 21-bit value to 64-bit
    int64_t imm = (immhi << 2) | immlo;
    if (imm & 0x100000) { // Check sign bit
        imm |= 0xFFFFFFFFFFE00000;
    }
    
    // ADRP: PC = (PC & ~0xFFF) + (imm << 12)
    uint64_t pc = reinterpret_cast<uint64_t>(base);
    uint64_t adrpAddr = (pc & ~0xFFFULL) + (imm << 12);
    
    // ADD instruction adds 12-bit immediate
    if (isADDInstruction(add)) {
        uint64_t imm12 = (add >> 10) & 0xFFF;
        adrpAddr += imm12;
    }
    
    return reinterpret_cast<void*>(adrpAddr);
}

size_t KittyArm64::getInstructionSize(uint32_t instruction) {
    // Most ARM64 instructions are 4 bytes
    // Check for compact instructions (T32, etc.) - not standard in ARM64
    return 4;
}

bool KittyArm64::readADRPValue(void* adrpAddr, int64_t& value) {
    if (!adrpAddr) {
        return false;
    }
    
    uint32_t* insns = static_cast<uint32_t*>(adrpAddr);
    
    if (!isADRPInstruction(insns[0])) {
        return false;
    }
    
    // Calculate the address ADRP references
    void* target = calculateADRPAddress(adrpAddr, insns[0], insns[1]);
    if (!target) {
        return false;
    }
    
    // Read the value at that address
    value = *reinterpret_cast<int64_t*>(target);
    return true;
}

bool KittyArm64::readLDRValue(void* ldrAddr, int64_t& value) {
    if (!ldrAddr) {
        return false;
    }
    
    uint32_t* insn = static_cast<uint32_t*>(ldrAddr);
    
    if (!isLDRInstruction(*insn)) {
        return false;
    }
    
    // Decode LDR immediate offset
    uint32_t rt = *insn & 0x1F;
    uint32_t rn = (*insn >> 5) & 0x1F;
    uint64_t imm12 = (*insn >> 10) & 0xFFF;
    
    // Read the base register value (we can't actually do this from here)
    // This is a simplified version
    value = static_cast<int64_t>(imm12);
    return true;
}

std::string KittyArm64::disassemble(uint32_t instruction, void* pc) {
    char buf[256];
    
    if (isNOP()) {
        snprintf(buf, sizeof(buf), "NOP");
    } else if (isBInstruction(instruction)) {
        int64_t offset = getBranchOffset(instruction);
        void* target = pc ? reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pc) + offset) : nullptr;
        snprintf(buf, sizeof(buf), "B %p", target);
    } else if (isBLInstruction(instruction)) {
        int64_t offset = getBranchOffset(instruction);
        void* target = pc ? reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pc) + offset) : nullptr;
        snprintf(buf, sizeof(buf), "BL %p", target);
    } else if (isADRPInstruction(instruction)) {
        snprintf(buf, sizeof(buf), "ADRP ...");
    } else if (isADDInstruction(instruction)) {
        snprintf(buf, sizeof(buf), "ADD ...");
    } else if (isLDRInstruction(instruction)) {
        snprintf(buf, sizeof(buf), "LDR ...");
    } else if (instruction == makeRet()) {
        snprintf(buf, sizeof(buf), "RET");
    } else {
        snprintf(buf, sizeof(buf), "0x%08X", instruction);
    }
    
    return std::string(buf);
}

} // namespace FirstPersonModel
