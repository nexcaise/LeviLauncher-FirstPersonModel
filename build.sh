#!/bin/bash

# Build script for FirstPersonModel mod
# Usage: ./build.sh [Release|Debug]

set -e

# Configuration
BUILD_TYPE="${1:-Release}"
ANDROID_ABI="arm64-v8a"
ANDROID_PLATFORM="android-26"
BUILD_DIR="build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Building FirstPersonModel${NC}"
echo -e "${GREEN}Build Type: ${BUILD_TYPE}${NC}"
echo -e "${GREEN}ABI: ${ANDROID_ABI}${NC}"
echo -e "${GREEN}========================================${NC}"

# Check for NDK
if [ -z "$ANDROID_NDK" ]; then
    if [ -n "$ANDROID_SDK" ]; then
        ANDROID_NDK="$ANDROID_SDK/ndk/27.2.12479018"
    else
        echo -e "${RED}Error: ANDROID_NDK not set${NC}"
        echo "Please set ANDROID_NDK environment variable or ANDROID_SDK"
        exit 1
    fi
fi

if [ ! -d "$ANDROID_NDK" ]; then
    echo -e "${RED}Error: NDK not found at $ANDROID_NDK${NC}"
    exit 1
fi

echo -e "${YELLOW}Using NDK: $ANDROID_NDK${NC}"

# Navigate to C++ source directory
cd "$(dirname "$0")/src/main/cpp"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_ARCH_ABI="$ANDROID_ABI" \
    -DCMAKE_ANDROID_NDK="$ANDROID_NDK" \
    -DCMAKE_SYSTEM_VERSION="${ANDROID_PLATFORM#android-}" \
    -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake"

# Build
echo -e "${YELLOW}Building...${NC}"
cmake --build . --parallel "$(nproc)"

# Check if build succeeded
if [ -f "../libs/${ANDROID_ABI}/libFirstPersonModel.so" ]; then
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}Build successful!${NC}"
    echo -e "${GREEN}Output: src/main/cpp/libs/${ANDROID_ABI}/libFirstPersonModel.so${NC}"
    echo -e "${GREEN}========================================${NC}"
else
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}Build failed!${NC}"
    echo -e "${RED}========================================${NC}"
    exit 1
fi
