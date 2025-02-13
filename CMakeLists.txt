cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)
project(CSC8508)

set(CMAKE_CXX_STANDARD 20)
set(FMOD_VERSION 2.02.26 CACHE STRING "" FORCE)

option(USE_SDL2 "Use SDL2 for window creation and input" $<NOT:$<BOOL:WIN32>>)

if (USE_SDL2)
    add_definitions(-DCSC_USE_SDL2)
endif()

if(MSVC)
    find_package(Bullet CONFIG REQUIRED)
    find_package(nlohmann_json CONFIG REQUIRED)
else()
    find_package(Bullet REQUIRED)
    find_package(nlohmann_json REQUIRED)
endif()

if (MSVC)
    include("${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

add_subdirectory(NCLCoreClasses)
add_subdirectory(MeshCompiler)
add_subdirectory(CSC8503CoreClasses)
add_subdirectory(OpenGLRendering)
add_subdirectory(Network)
add_subdirectory(fmod-cmake)
add_subdirectory(TeamProject) # Working title
