set(SOURCES
    BulletDebug.cpp
    BulletDebug.h
    GameTechRendererInterface.h
    TutorialGame.cpp
    TutorialGame.h
    PlayerController.cpp
    PlayerController.h
    PlayerObject.h
    Turret.h
    Turret.cpp
	BWHelperFunctions.h
	BWHelperFunctions.cpp
    LevelImporter.cpp
    LevelImporter.h
	NavMesh.h
	NavMesh.cpp
    ResourceManager.h
    ResourceManager.cpp
    Profiler.h
    Profiler.cpp
	NavEntity.h
	NavEntity.cpp
	AudioEngine.h
	AudioEngine.cpp
    Config.h
    Config.cpp
    PushdownState.h
    PushdownMachine.cpp
    PushdownMachine.h
    MainMenuState.h
    PlayingState.h
    PausedState.h
    
    Multiplayer/GamePackets.hpp
    Multiplayer/GamePacketHandlers.hpp
    Multiplayer/GamePacketHandlers.cpp
	Wanderer.h
	Wanderer.cpp
)

set(ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets")
file(GLOB_RECURSE AUDIO "${ASSET_ROOT}/Audio/*.wav")
file(GLOB_RECURSE DATA "${ASSET_ROOT}/Data/*")
file(GLOB_RECURSE FONTS "${ASSET_ROOT}/Fonts/*.fnt")
file(GLOB_RECURSE LEVELS "${ASSET_ROOT}/Levels/*.json")
file(GLOB_RECURSE MESHES "${ASSET_ROOT}/Meshes/*.msh")
if (PS5_BUILD)
    file(GLOB_RECURSE SHADERS "${ASSET_ROOT}/Shaders/PSSL/*.pssl")
else()
    file(GLOB_RECURSE SHADERS
        "${ASSET_ROOT}/Shaders/*.frag"
        "${ASSET_ROOT}/Shaders/*.vert"
    )
endif()
file(GLOB_RECURSE TEXTURES
    "${ASSET_ROOT}/Textures/*.jpg"
    "${ASSET_ROOT}/Textures/*.png"
    "${ASSET_ROOT}/Textures/*.dds"
)
set(ASSETS
    ${AUDIO}
    ${DATA}
    ${FONTS}
    ${LEVELS}
    ${MESHES}
    ${SHADERS}
    ${TEXTURES}
)

if(NOT PS5_BUILD)
    list(APPEND SOURCES
        main.cpp
        GameTechRenderer.cpp
        GameTechRenderer.h
    )
    add_executable(${PROJECT_NAME} ${SOURCES})
else()
    # There is quite a bit of extra work for PS5 builds,
    # so this is split into PS5Starter where we link against
    # the main game statically
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()
copy_assets(${RUNTIME_ASSET_PATH} ${ASSET_ROOT} ${PROJECT_NAME} ${ASSETS})

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
    "../NCLCoreClasses/Plane.h"
    "../NCLCoreClasses/Matrix.h"
    "../NCLCoreClasses/GameTimer.h"
)

################################################################################
# Dependencies
################################################################################
if(MSVC)
    # Required for enet
    target_link_libraries(${PROJECT_NAME} PUBLIC "Winmm.lib")
endif()

set(INCLUDE_DIRECTORIES
	${CMAKE_SOURCE_DIR}/OpenGLRendering/
	${CMAKE_SOURCE_DIR}/NCLCoreClasses/
	${CMAKE_SOURCE_DIR}/CSC8503CoreClasses/
    ${CMAKE_SOURCE_DIR}/TeamProject/
    ${CMAKE_SOURCE_DIR}/Network/
    ${CMAKE_SOURCE_DIR}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRECTORIES}>)

target_link_libraries(${PROJECT_NAME} PUBLIC NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} PUBLIC CSC8503CoreClasses)
if(NOT PS5_BUILD)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenGLRendering)
    # TODO: PS5 Support
    target_link_libraries(${PROJECT_NAME} PUBLIC NetworkLib)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC ${BULLET_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC fmod)
target_link_libraries(${PROJECT_NAME} PUBLIC fmodstudio)
fmod_copy_libs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIRS})

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set(outdir ${CMAKE_CURRENT_BINARY_DIR})
    # Don't create an extra `Debug/Release` directory for executables
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${outdir})
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${outdir})
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${outdir})
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${outdir})
endif()
