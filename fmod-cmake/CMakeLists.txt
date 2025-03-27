cmake_minimum_required(VERSION 3.22)
project(fmod_cmake)

set(FMOD_CMAKE_ROOT ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "" FORCE)

# ===== CMake Modules =====
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

# Provide functions
include(fmod_copy_libs)

# ===== FMOD =====
if (NOT FMOD_VERSION)
    set(FMOD_VERSION 2.02.21)
endif()

if (NOT FMOD_SONAME)
    set(FMOD_SONAME 14)
endif()

add_subdirectory(fmod)
