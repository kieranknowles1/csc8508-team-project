set(CMAKE_CONFIGURATION_TYPES "Debug;Release;NetworkTest" CACHE STRING "Build Configurations" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_NETWORKTEST "" CACHE STRING "Linker flags for NetworkTest Configuation")

set(SOURCES
    GameTechRenderer.cpp
    GameTechRenderer.h
    Main.cpp
    NetworkedGame.cpp
    NetworkedGame.h
    NetworkPlayer.cpp
    NetworkPlayer.h
    StateGameObject.cpp
    StateGameObject.h
    TutorialGame.cpp
    TutorialGame.h
)

set(NETWORK_TEST_SOURCES
	network_test.cpp
	network.cpp
)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
	$<$<CONFIG:Debug>:${SOURCES}>
	$<$<CONFIG:Release>:${SOURCES}>
	$<$<CONFIG:NetworkTest>:${NETWORK_TEST_SOURCES}>
)


set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

################################################################################
# Compile definitions
################################################################################
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "UNICODE;"
        "_UNICODE"
        "WIN32_LEAN_AND_MEAN"
        "_WINSOCKAPI_"
        "_WINSOCK2API_"
        "_WINSOCK_DEPRECATED_NO_WARNINGS"
    )
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vector>
    <map>
    <stack>
    <list>
    <set>
    <string>
    <thread>
    <atomic>
    <functional>
    <iostream>
    <chrono>
    <sstream>

    "../NCLCoreClasses/Vector.h"
    "../NCLCoreClasses/Quaternion.h"
    "../NCLCoreClasses/Plane.h"
    "../NCLCoreClasses/Matrix.h"
    "../NCLCoreClasses/GameTimer.h"
)

################################################################################
# Dependencies
################################################################################
if(MSVC)
    # Required for enet
    target_link_libraries(${PROJECT_NAME} "Winmm.lib")
endif()

set(INCLUDE_DIRECTORIES
	${CMAKE_SOURCE_DIR}/OpenGLRendering/
	${CMAKE_SOURCE_DIR}/NCLCoreClasses/
	${CMAKE_SOURCE_DIR}/CSC8503CoreClasses/
)

target_include_directories(${PROJECT_NAME} PRIVATE
	$<$<CONFIG:Debug>:${INCLUDE_DIRECTORIES}>
	$<$<CONFIG:Release>:${INCLUDE_DIRECTORIES}>
	$<$<CONFIG:NetworkTest>:${CMAKE_SOURCE_DIR}/CSC8503CoreClasses/>
	$<$<CONFIG:NetworkTest>:${CMAKE_SOURCE_DIR}/NCLCoreClasses/>
)

target_link_libraries(${PROJECT_NAME} 
	$<$<CONFIG:Debug>:NCLCoreClasses>
	$<$<CONFIG:Debug>:CSC8503CoreClasses>
	$<$<CONFIG:Debug>:OpenGLRendering>

	$<$<CONFIG:Release>:NCLCoreClasses>
	$<$<CONFIG:Release>:CSC8503CoreClasses>
	$<$<CONFIG:Release>:OpenGLRendering>

	$<$<CONFIG:NetworkTest>:CSC8503CoreClasses>
	$<$<CONFIG:NetworkTest>:NCLCoreClasses>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:NetworkTest>:NETWORK_TEST>
)

target_compile_options(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:NetworkTest>:-O2>
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
