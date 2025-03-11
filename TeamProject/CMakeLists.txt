set(SOURCES
    main.cpp
    BulletDebug.cpp
    BulletDebug.h
    GameTechRendererInterface.cpp
    GameTechRendererInterface.h
    TutorialGame.cpp
    TutorialGame.h
    PlayerController.cpp
    PlayerController.h
    PlayerObject.h
    PlayerObject.cpp
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
    DecalSystem.h
    DecalSystem.cpp
    MainMenuScreen.h
    GameScreen.h
    PauseScreen.h
    Colors.h
    Multiplayer/GamePackets.hpp
    Multiplayer/GamePacketHandlers.hpp
    Multiplayer/GamePacketHandlers.cpp
    Multiplayer/Lobby.hpp
    Multiplayer/Lobby.cpp
    Multiplayer/User.hpp
	Wanderer.h
	Wanderer.cpp
    Shoot.cpp
    Shoot.h
    Paintball.h
    Paintball.cpp
    PointLight.h
    Respawn.h
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
        GameTechRenderer.cpp
        GameTechRenderer.h
    )
else()
    list(APPEND SOURCES
        GameTechAGCRenderer.h
        GameTechAGCRenderer.cpp
    )
endif()

add_executable(${PROJECT_NAME} ${SOURCES})
copy_assets(${RUNTIME_ASSET_PATH} ${ASSET_ROOT} ${PROJECT_NAME} ${ASSETS})

################################################################################
# Compile definitions
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIRS})

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

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
	VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)"
)
################################################################################
# Dependencies
################################################################################
if(MSVC)
    # Required for enet
    target_link_libraries(${PROJECT_NAME} PUBLIC "Winmm.lib")

    target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
    )
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
target_link_libraries(${PROJECT_NAME} PUBLIC NetworkLib)
if(NOT PS5_BUILD)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenGLRendering)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC PS5Core)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC ${BULLET_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC fmod)
target_link_libraries(${PROJECT_NAME} PUBLIC fmodstudio)
fmod_copy_libs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIRS})

set(SCE_MODULES
    libc.prx
    libSceFace.prx
    libSceFaceTracker.prx
    libSceFontGsm.prx
    libSceJobManager.prx
    libSceJobManager_nosubmission.prx
    libSceNpCppWebApi.prx
    libScePfs.prx
)

set(SCE_DEPENDENCIES
    SceAgcDriver_stub_weak
    SceAgc_stub_weak
    SceVideoOut_stub_weak
    SceAgc_debug_nosubmission
    SceAgcCore_debug_nosubmission
    SceAgcGpuAddress_debug_nosubmission

    ScePad_stub_weak
    SceUserService_stub_weak
    SceNpUtility_stub_weak
    SceCommonDialog_stub_weak
    SceNetCtlApDialog_stub_weak
    SceNetCtlAp_stub_weak
    SceNetCtl_stub_weak
    SceNet_stub_weak
    ScePosix_stub_weak
)

if(PS5_BUILD)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${SCE_DEPENDENCIES})

    foreach(file ${SCE_MODULES})
        file(COPY ${PS5_SCE_MODULE}/${file} DESTINATION ${CMAKE_BINARY_DIR}/sce_module/)
    endforeach()
endif()

include_directories(${COMPILE_ASSET_ROOT})

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
