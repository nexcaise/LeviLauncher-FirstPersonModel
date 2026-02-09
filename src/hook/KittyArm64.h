#pragma once

#include <cstdint>
#include <string>

namespace FirstPersonModel {

// ARM64 specific utilities
class KittyArm64 {
public:
    // Decode ARM64 instruction
    static bool decodeInstruction(uint32_t instruction, uint32_t& opcode, uint32_t& rd, uint32_t& rn, uint32_t& rm);
    
    // Create ARM64 instructions
    static uint32_t makeBranchInstruction(int64_t offset);
    static uint32_t makeAbsoluteBranch(void* target);
    static uint32_t makeNOP();
    static uint32_t makeRet();
    static uint32_t makeMovRegister(uint32_t rd, uint32_t rn);
    static uint32_t makeMovImmediate(uint32_t rd, uint16_t imm, uint32_t shift = 0);
    
    // Check instruction type
    static bool isBranchInstruction(uint32_t instruction);
    static bool isBLInstruction(uint32_t instruction);
    static bool isBInstruction(uint32_t instruction);
    static bool isCBZInstruction(uint32_t instruction);
    static bool isTBZInstruction(uint32_t instruction);
    static bool isADRPInstruction(uint32_t instruction);
    static bool isADDInstruction(uint32_t instruction);
    static bool isLDRInstruction(uint32_t instruction);
    
    // Get branch target
    static int64_t getBranchOffset(uint32_t instruction);
    static void* calculateADRPAddress(void* base, uint32_t adrp, uint32_t add);
    
    // Instruction utilities
    static size_t getInstructionSize(uint32_t instruction);
    static std::string disassemble(uint32_t instruction, void* pc = nullptr);
    
    // Memory operations
    static bool readADRPValue(void* adrpAddr, int64_t& value);
    static bool readLDRValue(void* ldrAddr, int64_t& value);
};

// ARM64 registers
enum class Arm64Reg : uint32_t {
    X0 = 0, X1, X2, X3, X4, X5, X6, X7,
    X8, X9, X10, X11, X12, X13, X14, X15,
    X16, X17, X18, X19, X20, X21, X22, X23,
    X24, X25, X26, X27, X28, X29, X30,
    SP = 31, XZR = 31
};

} // namespace FirstPersonModel
