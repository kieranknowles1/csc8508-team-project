cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)
project(CSC8508)

set(CMAKE_CXX_STANDARD 20)

option(USE_SDL2 "Use SDL2 for window creation and input" $<NOT:$<BOOL:WIN32>>)

if (USE_SDL2)
    add_definitions(-DCSC_USE_SDL2)
endif()

set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "Don't build bullet demos")
set(BUILD_EXTRAS OFF CACHE BOOL "Don't build bullet extras")
set(BUILD_UNIT_TESTS OFF CACHE BOOL "Don't build bullet tests")
set(BULLET_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ThirdParty/bullet3/src")

add_subdirectory(ThirdParty/bullet3)
add_subdirectory(ThirdParty/nlohmann_json)

add_subdirectory(NCLCoreClasses)
add_subdirectory(MeshCompiler)
add_subdirectory(CSC8503CoreClasses)
add_subdirectory(OpenGLRendering)
add_subdirectory(Network)
add_subdirectory(TeamProject) # Working title
