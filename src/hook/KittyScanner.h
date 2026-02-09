#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace FirstPersonModel {

// Pattern scanning utilities
class KittyScanner {
public:
    // Pattern scan in range
    static void* findPatternInRange(const void* start, size_t size, const std::string& pattern, const std::string& mask);
    static void* findPatternInRange(const void* start, size_t size, const std::string& pattern);
    
    // Pattern scan in library
    static void* findPatternInLibrary(const char* libraryName, const std::string& pattern, const std::string& mask);
    static void* findPatternInLibrary(const char* libraryName, const std::string& pattern);
    
    // IDA-style pattern scan (e.g., "48 8B ? ? ? 89")
    static void* findIdaPattern(const void* start, size_t size, const std::string& idaPattern);
    static void* findIdaPatternInLibrary(const char* libraryName, const std::string& idaPattern);
    
    // Byte pattern to mask conversion
    static bool parseIdaPattern(const std::string& idaPattern, std::vector<uint8_t>& bytes, std::string& mask);
    
    // Scan with wildcard support
    // Pattern format: "AB CD ?? EF" where ?? is a wildcard
    static void* scan(const void* start, size_t size, const std::string& pattern);
    
private:
    static bool comparePattern(const uint8_t* data, const std::vector<uint8_t>& pattern, const std::string& mask);
};

// Signature scanner helper
class SignatureScanner {
public:
    SignatureScanner(const char* libraryName);
    
    void* scan(const std::string& pattern);
    void* scan(const std::vector<uint8_t>& pattern, const std::string& mask);
    
    template<typename T>
    T* scanAndCast(const std::string& pattern) {
        return reinterpret_cast<T*>(scan(pattern));
    }
    
private:
    const char* m_libraryName;
    void* m_base = nullptr;
    size_t m_size = 0;
};

// Utility functions
namespace KittyUtils {
    // Hex string to bytes
    std::vector<uint8_t> hexToBytes(const std::string& hex);
    std::string bytesToHex(const std::vector<uint8_t>& bytes);
    
    // String utilities
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    
    // Address formatting
    std::string addressToHex(void* address);
    void* hexToAddress(const std::string& hex);
}

} // namespace FirstPersonModel
