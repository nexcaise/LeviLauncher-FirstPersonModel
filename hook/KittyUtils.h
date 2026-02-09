#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace FirstPersonModel {
namespace KittyUtils {

// Hex string utilities
std::vector<uint8_t> hexToBytes(const std::string& hex);
std::string bytesToHex(const std::vector<uint8_t>& bytes);
std::string bytesToHex(const uint8_t* bytes, size_t len);

// String utilities
std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);
std::vector<std::string> split(const std::string& str, const std::string& delimiter);
std::string toLower(const std::string& str);
std::string toUpper(const std::string& str);
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::string& str, const std::string& suffix);

// Address utilities
std::string addressToHex(void* address);
void* hexToAddress(const std::string& hex);
template<typename T>
std::string pointerToHex(T* ptr) {
    return addressToHex(reinterpret_cast<void*>(ptr));
}

// Number utilities
bool isHexString(const std::string& str);
bool isNumber(const std::string& str);
int64_t hexToInt(const std::string& hex);
std::string intToHex(int64_t value);

// Pattern utilities
std::string normalizePattern(const std::string& pattern);
bool validatePattern(const std::string& pattern);

// File utilities
bool fileExists(const std::string& path);
std::string readFile(const std::string& path);
bool writeFile(const std::string& path, const std::string& content);
std::vector<uint8_t> readBinaryFile(const std::string& path);
bool writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data);

// Time utilities
uint64_t getCurrentTimeMillis();
void sleepMillis(uint64_t millis);

} // namespace KittyUtils
} // namespace FirstPersonModel
