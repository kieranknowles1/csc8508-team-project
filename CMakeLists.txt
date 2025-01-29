cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)
project(CSC8508)

set(CMAKE_CXX_STANDARD 20)

option(USE_SDL2 "Use SDL2 for window creation and input" $<NOT:$<BOOL:WIN32>>)

if (USE_SDL2)
    add_definitions(-DCSC_USE_SDL2)
endif()

add_subdirectory(NCLCoreClasses)
add_subdirectory(CSC8503CoreClasses)
add_subdirectory(OpenGLRendering)
add_subdirectory(TeamProject) # Working title
