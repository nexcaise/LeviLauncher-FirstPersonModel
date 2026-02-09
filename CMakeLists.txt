cmake_minimum_required(VERSION 3.18)
project(FirstPersonModel VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT ANDROID)
    message(FATAL_ERROR "This mod is designed for Android only (LeviLaunchroid)")
endif()

set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options(
    -O2
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fno-rtti
    -fno-exceptions
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-unused-variable
)

add_definitions(-DANDROID -DFIRSTPERSONMODEL_EXPORTS)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/hook
    ${CMAKE_CURRENT_SOURCE_DIR}/patch
)

set(SOURCES
    src/FirstPersonModel.cpp
    src/ModMain.cpp
    src/CameraHooks.cpp
    src/PlayerRenderer.cpp
    hook/HookManager.cpp
    hook/ARMPatch.cpp
    hook/KittyMemory.cpp
    hook/KittyUtils.cpp
    hook/KittyScanner.cpp
    hook/KittyArm64.cpp
)

set(HEADERS
    include/FirstPersonModel.h
    include/CameraAPI.h
    include/ICameraAPI.h
    include/Signatures.h
    include/Offsets.h
    include/Logger.h
    hook/HookManager.h
    hook/KittyMemory.h
    hook/KittyUtils.h
    hook/KittyScanner.h
    hook/KittyArm64.h
    hook/ARMPatch.h
)

add_library(FirstPersonModel SHARED ${SOURCES} ${HEADERS})

find_library(log-lib log)
find_library(GLESV2_LIB GLESv2)

if(${CMAKE_ANDROID_ARCH_ABI} STREQUAL "arm64-v8a")
    target_compile_definitions(FirstPersonModel PRIVATE ARM64)
    set(ARCH_DIR arm64)
elseif(${CMAKE_ANDROID_ARCH_ABI} STREQUAL "armeabi-v7a")
    target_compile_definitions(FirstPersonModel PRIVATE ARMV7)
    set(ARCH_DIR arm)
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_ANDROID_ARCH_ABI}")
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH_DIR}/libsubstrate.a)
    set(SUBSTRATE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH_DIR}/libsubstrate.a)
else()
    message(WARNING "Substrate library not found. Using inline hooking only.")
    set(SUBSTRATE_LIB "")
endif()

target_link_libraries(FirstPersonModel
    ${log-lib}
    ${GLESV2_LIB}
    ${SUBSTRATE_LIB}
    dl
)

set_target_properties(FirstPersonModel PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/${CMAKE_ANDROID_ARCH_ABI}
)

add_custom_command(TARGET FirstPersonModel POST_BUILD
    COMMAND ${CMAKE_STRIP} $<TARGET_FILE:FirstPersonModel>
    COMMENT "Stripping symbols from FirstPersonModel.so"
)

message(STATUS "Building FirstPersonModel for ${CMAKE_ANDROID_ARCH_ABI}")
message(STATUS "NDK Version: ${CMAKE_ANDROID_NDK_VERSION}")
message(STATUS "API Level: ${CMAKE_SYSTEM_VERSION}")
