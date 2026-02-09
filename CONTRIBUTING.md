# Contributing to FirstPersonModel

Thank you for your interest in contributing to FirstPersonModel! This document provides guidelines for contributing to this project.

## Code of Conduct

This project and everyone participating in it is governed by our code of conduct: be respectful, constructive, and inclusive.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check the existing issues to see if the problem has already been reported. When you are creating a bug report, please include as many details as possible:

- **Use a clear and descriptive title**
- **Describe the exact steps to reproduce the problem**
- **Provide specific examples to demonstrate the steps**
- **Describe the behavior you observed and what behavior you expected**
- **Include information about your configuration:**
  - Minecraft version
  - Android version
  - Device model
  - LeviLaunchroid version

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion, please include:

- **Use a clear and descriptive title**
- **Provide a step-by-step description of the suggested enhancement**
- **Provide specific examples to demonstrate the enhancement**
- **Explain why this enhancement would be useful**

### Pull Requests

1. Fork the repository
2. Create a branch for your feature (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Run the code formatter (`clang-format -i src/main/cpp/**/*.cpp src/main/cpp/**/*.h`)
5. Commit your changes (`git commit -m 'Add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## Development Setup

### Prerequisites

- Android NDK r27b or later
- CMake 3.22+
- Android SDK (API 26+)
- Java JDK 21+

### Building

```bash
cd src/main/cpp
mkdir build && cd build
cmake .. \
  -DCMAKE_SYSTEM_NAME=Android \
  -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
  -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
  -DCMAKE_SYSTEM_VERSION=26 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

## Style Guidelines

### C++ Code Style

- Use 4 spaces for indentation
- Use `camelCase` for functions and variables
- Use `PascalCase` for classes and structs
- Use `ALL_CAPS` for macros and constants
- Place opening braces on the same line
- Maximum line length: 120 characters

Example:
```cpp
class MyClass {
public:
    void myFunction(int parameter) {
        if (condition) {
            doSomething();
        }
    }
    
private:
    int m_memberVariable;
};
```

### Naming Conventions

- **Files**: `PascalCase.cpp`, `PascalCase.h`
- **Classes**: `PascalCase`
- **Functions**: `camelCase`
- **Variables**: `camelCase`
- **Member variables**: `m_camelCase`
- **Constants**: `kPascalCase` or `ALL_CAPS`
- **Namespaces**: `lowercase`

## Testing

- Test your changes on multiple Android versions if possible
- Test with different Minecraft versions
- Ensure the mod doesn't crash when disabled
- Check that hooks are properly cleaned up

## Documentation

- Update the README.md if you change functionality
- Add comments for complex code sections
- Document public APIs with doxygen-style comments

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Questions?

Feel free to open an issue with your question or join our community discussions.

Thank you for contributing!
