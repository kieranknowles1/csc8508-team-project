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

add_executable(${PROJECT_NAME} ${SOURCES})

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

include_directories("../OpenGLRendering/")
include_directories("../NCLCoreClasses/")
include_directories("../CSC8503CoreClasses/")

target_link_libraries(${PROJECT_NAME} NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} CSC8503CoreClasses)
target_link_libraries(${PROJECT_NAME} OpenGLRendering)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
