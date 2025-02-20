cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(CSC8508)

set(CMAKE_CXX_STANDARD 20)


option(USE_SDL2 "Use SDL2 for window creation and input" OFF)
option(PS5_BUILD "Build for the PS5, also needs the options passed through configure-ps5" OFF)

if (USE_SDL2)
    add_definitions(-DCSC_USE_SDL2)
endif()

if (PS5_BUILD)
    add_compile_definitions("USEAGC")
endif()

if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

################################################################################
# Third-party dependencies
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

################################################################################
# Projects
################################################################################
set(ASSETS_ROOT ${CMAKE_SOURCE_DIR}/Assets)

file(GLOB_RECURSE MESHES "${ASSETS_ROOT}/Meshes/*.msh")
if(NOT PS5_BUILD)
    file(GLOB_RECURSE SHADERS
        "${ASSETS_ROOT}/Shaders/*.frag"
        "${ASSETS_ROOT}/Shaders/*.vert"
    )
else()
    file(GLOB_RECURSE SHADERS
        "${ASSETS_ROOT}/Shaders/PSSL/*.pssl"
    )
endif()
file(GLOB_RECURSE TEXTURES
    "${ASSETS_ROOT}/Textures/*.jpg"
    "${ASSETS_ROOT}/Textures/*.png"
    "${ASSETS_ROOT}/Textures/*.dds"
    # FIXME: These aren't being converted properly on PS5 due to only having
    # 1 channel
    # "${ASSETS_ROOT}/Textures/*.tga"
)
file(GLOB_RECURSE LEVELS "${ASSETS_ROOT}/Levels/*")
file(GLOB_RECURSE DATA "${ASSETS_ROOT}/Data/*")
file(GLOB_RECURSE FONTS "${ASSETS_ROOT}/Fonts/*")

add_subdirectory(NCLCoreClasses)
# TODO: Can we do this on PS5 as well?
if (NOT PS5_BUILD)
    add_subdirectory(MeshCompiler)
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
