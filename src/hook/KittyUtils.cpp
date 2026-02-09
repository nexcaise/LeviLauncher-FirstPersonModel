#include "KittyUtils.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

namespace FirstPersonModel {
namespace KittyUtils {

std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    std::string trimmed = hex;
    
    // Remove spaces
    trimmed.erase(std::remove(trimmed.begin(), trimmed.end(), ' '), trimmed.end());
    
    for (size_t i = 0; i < trimmed.length(); i += 2) {
        if (i + 1 < trimmed.length()) {
            unsigned int byte;
            sscanf(trimmed.substr(i, 2).c_str(), "%02x", &byte);
            bytes.push_back(static_cast<uint8_t>(byte));
        }
    }
    
    return bytes;
}

std::string bytesToHex(const std::vector<uint8_t>& bytes) {
    std::string hex;
    for (uint8_t b : bytes) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", b);
        hex += buf;
    }
    return hex;
}

std::string bytesToHex(const uint8_t* bytes, size_t len) {
    std::string hex;
    for (size_t i = 0; i < len; i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", bytes[i]);
        hex += buf;
    }
    return hex;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    
    tokens.push_back(str.substr(start));
    return tokens;
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string addressToHex(void* address) {
    char buf[20];
    snprintf(buf, sizeof(buf), "%p", address);
    return std::string(buf);
}

void* hexToAddress(const std::string& hex) {
    uintptr_t addr;
    sscanf(hex.c_str(), "%lx", &addr);
    return reinterpret_cast<void*>(addr);
}

bool isHexString(const std::string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!isxdigit(c) && c != ' ' && c != '?') {
            return false;
        }
    }
    return true;
}

bool isNumber(const std::string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!isdigit(c) && c != '-' && c != '.') {
            return false;
        }
    }
    return true;
}

int64_t hexToInt(const std::string& hex) {
    int64_t value;
    sscanf(hex.c_str(), "%lx", &value);
    return value;
}

std::string intToHex(int64_t value) {
    char buf[20];
    snprintf(buf, sizeof(buf), "%lx", value);
    return std::string(buf);
}

std::string normalizePattern(const std::string& pattern) {
    std::string normalized;
    for (char c : pattern) {
        if (c != ' ') {
            normalized += c;
        }
    }
    return normalized;
}

bool validatePattern(const std::string& pattern) {
    std::string normalized = normalizePattern(pattern);
    if (normalized.length() % 2 != 0) {
        return false;
    }
    
    for (size_t i = 0; i < normalized.length(); i += 2) {
        std::string byte = normalized.substr(i, 2);
        if (byte != "??") {
            for (char c : byte) {
                if (!isxdigit(c)) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    return file.good();
}

std::vector<uint8_t> readBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }
    
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});
}

bool writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return file.good();
}

uint64_t getCurrentTimeMillis() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

void sleepMillis(uint64_t millis) {
    usleep(millis * 1000);
}

} // namespace KittyUtils
} // namespace FirstPersonModel
