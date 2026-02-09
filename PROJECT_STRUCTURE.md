# FirstPersonModel Project Structure

This document outlines the complete structure of the FirstPersonModel mod for LeviLaunchroid.

## Directory Structure

```
FirstPersonModel/
├── .github/
│   └── workflows/
│       ├── build.yml          # Main CI/CD workflow
│       └── lint.yml           # Code quality checks
├── src/
│   └── main/
│       └── cpp/
│           ├── CMakeLists.txt           # Main CMake configuration
│           ├── include/                 # Header files
│           │   ├── Logger.h             # Logging utilities
│           │   ├── FirstPersonModel.h   # Main mod class
│           │   ├── CameraAPI.h          # Camera API interface
│           │   ├── Signatures.h         # Memory signatures
│           │   └── Offsets.h            # Memory offsets
│           ├── src/                     # Implementation files
│           │   ├── ModMain.cpp          # Entry point
│           │   ├── FirstPersonModel.cpp # Main mod logic
│           │   ├── CameraHooks.cpp      # Camera hooking
│           │   ├── PlayerRenderer.cpp   # Player rendering
│           │   └── Offsets.cpp          # Offset management
│           └── hook/                    # Hooking framework
│               ├── ARMPatch.h/cpp       # ARM patching utilities
│               ├── KittyMemory.h/cpp    # Memory management
│               ├── KittyScanner.h/cpp   # Pattern scanning
│               ├── KittyArm64.h/cpp     # ARM64 utilities
│               ├── KittyUtils.h/cpp     # Utility functions
│               └── HookManager.h/cpp    # Hook management
├── manifest.json            # Mod manifest
├── CMakeLists.txt           # Root CMake (if building standalone)
├── build.sh                 # Build script
├── .clang-format            # Code formatting rules
├── .gitignore               # Git ignore rules
├── LICENSE                  # MIT License
├── CONTRIBUTING.md          # Contribution guidelines
└── README.md                # Project documentation
```

## Key Components

### 1. Entry Point (ModMain.cpp)
- Constructor function runs when library loads
- Creates initialization thread
- Waits for Minecraft library
- Initializes all systems

### 2. Core Mod (FirstPersonModel.cpp)
- Main mod singleton
- Manages enabled/disabled state
- Handles configuration
- Coordinates between systems

### 3. Camera System (CameraHooks.cpp)
- Hooks into Minecraft's camera rendering
- Applies camera offsets in first person
- Interfaces with CameraAPI

### 4. Player Rendering (PlayerRenderer.cpp)
- Forces player model to render in first person
- Modifies render flags
- Hooks player render function

### 5. Memory Framework
- **KittyMemory**: Safe memory read/write
- **KittyScanner**: Pattern/signature scanning
- **KittyArm64**: ARM64 instruction handling
- **ARMPatch**: Basic ARM patching
- **HookManager**: High-level hook management

## Build System

### CMake Configuration
- Targets Android ARM64 (arm64-v8a)
- Uses NDK r27b+
- Supports API 26+
- Builds as shared library (.so)

### GitHub Actions
- Automated builds on push/PR
- Multi-API level testing (26, 28, 30, 33)
- Automatic release creation on tags
- Code formatting checks (clang-format)

## Memory Hooking Architecture

### 1. Signature Scanning
```cpp
void* addr = KittyScanner::findIdaPatternInLibrary(
    "libminecraftpe.so",
    "48 ?? ?? ?? 21 ?? ?? 91 ?? ?? 00 ?? ?? 91 ?? ?? 40 ?? ?? 00 00 ?? ?? 00 94"
);
```

### 2. Function Hooking
```cpp
HookManager::getInstance().hook(
    targetAddress,
    replacementFunction,
    &originalFunction
);
```

### 3. Memory Patching
```cpp
KittyMemory::makeMemoryRWX(address, size);
KittyMemory::writeMemory(address, data, size);
```

## Signatures Used

### Camera::render
```
48 ?? ?? ?? 21 ?? ?? 91 ?? ?? 00 ?? ?? 91 ?? ?? 40 ?? ?? 00 00 ?? ?? 00 94
```

### Camera::isFirstPerson
```
FD 7B ?? A9 F4 4F ?? A9 FD 03 00 91 F3 03 00 ?? ?? ?? 00 ?? ?? ?? 00 94
```

### Player::render
```
FF 03 04 D1 FD 7B 0C A9 FA 67 0D A9 F8 5F 0E A9 F6 57 0F A9 F4 4F 10 A9
```

## Offsets (Version 1.20.x)

### Camera
- Position: 0x10
- Rotation: 0x20
- FOV: 0x30
- Perspective: 0x34

### Player
- Position: 0x2B0
- Rotation: 0x2C0

## API Integration

### CameraAPI Interface
```cpp
class ICameraAPI {
    virtual void getPosition(float* x, float* y, float* z) const = 0;
    virtual void setPosition(float x, float y, float z) = 0;
    virtual bool isFirstPerson() const = 0;
};
```

## Thread Safety

- Main initialization in separate thread
- Waits for library loading
- Thread-safe hook installation
- Synchronization through pthreads

## Error Handling

- Comprehensive logging via Android logcat
- Graceful degradation on signature failures
- Safe cleanup on shutdown
- NULL checks throughout

## Version Compatibility

The mod uses signature scanning for version-agnostic operation:
- Patterns work across Minecraft versions
- Dynamic offset resolution
- Fallback to default offsets if scanning fails

## Configuration

Future versions will support JSON configuration:
```json
{
  "enabled": true,
  "camera_offset_x": 0.0,
  "camera_offset_y": -0.1,
  "camera_offset_z": 0.3,
  "model_scale": 1.0
}
```

## Building

### Prerequisites
- Android NDK r27b+
- CMake 3.22+
- Android SDK API 26+

### Build Commands
```bash
# Using script
./build.sh Release

# Manual build
cd src/main/cpp
mkdir build && cd build
cmake .. -DCMAKE_SYSTEM_NAME=Android \
         -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
         -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
         -DCMAKE_SYSTEM_VERSION=26 \
         -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

## Installation

1. Build the mod
2. Copy `libFirstPersonModel.so` to LeviLaunchroid mods directory
3. Launch Minecraft through LeviLaunchroid
4. Mod automatically activates

## Debugging

### View Logs
```bash
adb logcat -s FirstPersonModel:D
```

### Check Library Loading
```bash
adb shell cat /proc/$(pidof com.mojang.minecraftpe)/maps | grep FirstPersonModel
```

## License

MIT License - See LICENSE file for details

## Contributing

See CONTRIBUTING.md for guidelines

## Support

- GitHub Issues
- LeviMC Discord: https://discord.gg/v5R5P4vRZk
