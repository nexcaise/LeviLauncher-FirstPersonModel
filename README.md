# FirstPersonModel for LeviLaunchroid

[![Build](https://github.com/FirstPersonModel/FirstPersonModel-LeviLaunchroid/actions/workflows/build.yml/badge.svg)](https://github.com/FirstPersonModel/FirstPersonModel-LeviLaunchroid/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-Android-green.svg)](https://www.android.com/)

A client-side mod for Minecraft Bedrock Edition on Android that displays the player model in first person view.

![First Person Model Preview](https://cdn.modrinth.com/data/H5XMjpHi/images/33868d5bc520137e440ced4ddc269e5a635f1466_350.webp)

## Features

- **First Person Model**: Shows your player model when in first person view
- **Camera Offset**: Adjusts camera position to avoid clipping with the model
- **Smooth Transitions**: Smooth camera movement when switching perspectives
- **Clientside Only**: Works without server modifications
- **Zero ImGui**: Lightweight implementation without external UI dependencies
- **LeviLaunchroid Compatible**: Designed specifically for LeviLaunchroid on Android

## Requirements

- Android 8.0+ (API 26+)
- ARM64 device (arm64-v8a)
- LeviLaunchroid launcher
- Legitimate copy of Minecraft Bedrock Edition

## Installation

1. Download the latest release from [Releases](https://github.com/FirstPersonModel/FirstPersonModel-LeviLaunchroid/releases)
2. Extract the zip file
3. Place `libFirstPersonModel.so` in your LeviLaunchroid mods directory
4. Launch Minecraft Bedrock through LeviLaunchroid
5. The mod will automatically activate

## Building from Source

### Prerequisites

- Android Studio or CMake 3.22+
- Android NDK (r27b or later)
- Android SDK (API 26+)
- Java JDK 21+

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/FirstPersonModel/FirstPersonModel-LeviLaunchroid.git
cd FirstPersonModel-LeviLaunchroid

# Navigate to the C++ source directory
cd src

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. \
  -DCMAKE_SYSTEM_NAME=Android \
  -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
  -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
  -DCMAKE_SYSTEM_VERSION=26 \
  -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --parallel

# Output will be in libs/arm64-v8a/libFirstPersonModel.so
```

## Technical Details

### Architecture

The mod uses the following approach:

1. **Memory Hooking**: Hooks into Minecraft's rendering pipeline using ARMPatch
2. **Camera API Integration**: Interfaces with `CameraAPI` and `ICameraAPI` classes
3. **VTable Manipulation**: Modifies virtual function tables for camera and player rendering
4. **Signature Scanning**: Uses pattern scanning to locate functions across versions

### Key Components

- `CameraHooks.cpp`: Hooks camera rendering functions
- `PlayerRenderer.cpp`: Modifies player model rendering
- `KittyMemory`: Memory patching and scanning library
- `ARMPatch`: ARM architecture hooking framework

### Signatures Used

The mod uses the following function signatures for cross-version compatibility:

```cpp
// Camera rendering function
"48 ?? ?? ?? 21 ?? ?? 91 ?? ?? 00 ?? ?? 91 ?? ?? 40 ?? ?? 00 00 ?? ?? 00 94"

// Player model rendering
"FD 7B ?? A9 F4 4F ?? A9 FD 03 00 91 F4 03 01 AA ?? ?? 00 ?? ?? ?? 00 94"
```

## API Documentation

### CameraAPI Interface

```cpp
class ICameraAPI {
public:
    virtual void setCameraPosition(const Vec3& pos) = 0;
    virtual void setCameraRotation(float pitch, float yaw, float roll) = 0;
    virtual bool isFirstPerson() const = 0;
    virtual void setFirstPersonModelVisible(bool visible) = 0;
};
```

### Configuration

The mod can be configured by creating a `firstpersonmodel.json` file:

```json
{
  "enabled": true,
  "camera_offset_x": 0.0,
  "camera_offset_y": -0.1,
  "camera_offset_z": 0.3,
  "model_scale": 1.0,
  "hide_in_water": true,
  "hide_when_sneaking": false
}
```

## Compatibility

| Minecraft Version | Status |
|------------------|--------|
| 1.20.x           | ✅ Supported |
| 1.21.x           | ✅ Supported |

## Troubleshooting

### Mod not loading
- Ensure you're using LeviLaunchroid
- Check that your device is ARM64
- Verify the .so file is in the correct directory

### Game crashes
- Make sure you're using a compatible Minecraft version
- Check logcat for error messages
- Ensure all dependencies are installed

## Contributing

Contributions are welcome! Please read our [Contributing Guidelines](CONTRIBUTING.md) before submitting PRs.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [LeviMC Team](https://github.com/LiteLDev) for LeviLaunchroid
- [KittyMemory](https://github.com/MJx0/KittyMemory) for memory patching utilities
- [ARMPatch](https://github.com/RusJJ/ARMPatch) for ARM hooking framework

## Disclaimer

This mod requires a legitimate copy of Minecraft Bedrock Edition. We do not support piracy.

## Support

- [GitHub Issues](https://github.com/FirstPersonModel/FirstPersonModel-LeviLaunchroid/issues)
- [LeviMC Discord](https://discord.gg/v5R5P4vRZk)

---

Made with ❤️ for the Minecraft Bedrock community
# LeviLauncher-FirstPersonModel
