cmake_minimum_required(VERSION 3.28.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(CSC8508)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
set(FMOD_VERSION 2.02.26 CACHE STRING "" FORCE)

option(USE_SDL2 "Use SDL2 for window creation and input" OFF)
option(PS5_BUILD "Build for the PS5, also needs the options passed through configure-ps5" OFF)
if (PS5_BUILD)
    set(PS5_SCE_MODULE ${CMAKE_SOURCE_DIR}/../sce_module CACHE STRING "Location of the `sce_module` directory")

    set(FMOD_SDK_PATH "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API PS5/api" CACHE STRING "Path to the FMOD SDK")
    set(PS5_SDK_TOOLS "C:/Program Files (x86)/SCE/Prospero SDKs/10.000/host_tools/bin")
else()
    set(FMOD_SDK_PATH "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api" CACHE STRING "Path to the FMOD SDK")
endif()

if (USE_SDL2)
    add_definitions(-DCSC_USE_SDL2)
endif()

if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if (PS5_BUILD)
    set(RUNTIME_ASSET_PATH ${CMAKE_BINARY_DIR}/Assets)
else()
    set(RUNTIME_ASSET_PATH ${CMAKE_BINARY_DIR}/TeamProject/Assets)
endif()

include(copy_assets)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(COMPILE_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets/" CACHE STRING "")

set(DEBUG_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets/" CACHE STRING "")
set(RELEASE_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets/" CACHE STRING "")

add_compile_definitions(ASSETROOTLOCATION="${DEBUG_ASSET_ROOT}")

if(MSVC)
    add_compile_definitions("NOMINMAX")
    add_compile_definitions("WIN32_LEAN_AND_MEAN")
endif()

set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "Don't build bullet demos")
set(BUILD_EXTRAS OFF CACHE BOOL "Don't build bullet extras")
set(BUILD_UNIT_TESTS OFF CACHE BOOL "Don't build bullet tests")
set(BULLET_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ThirdParty/bullet3/src")
set(BULLET_LIBRARIES BulletDynamics BulletCollision LinearMath)

add_subdirectory(ThirdParty/bullet3)
add_subdirectory(ThirdParty/nlohmann_json)

add_subdirectory(fmod-cmake)
include_directories(
    ${FMOD_SDK_PATH}/core/inc
    ${FMOD_SDK_PATH}/studio/inc
)

add_subdirectory(NCLCoreClasses)
if (NOT PS5_BUILD)
    add_subdirectory(MeshCompiler)
else()
    add_executable(MeshCompiler IMPORTED)
    set_target_properties(MeshCompiler PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/build/MeshCompiler/Debug/MeshCompiler)
endif()
add_subdirectory(CSC8503CoreClasses)
if (NOT PS5_BUILD)
    add_subdirectory(OpenGLRendering)
else()
    add_subdirectory(PS5Core)
    add_subdirectory(PS5Starter)
endif()
add_subdirectory(Network)
add_subdirectory(TeamProject) # Working title
