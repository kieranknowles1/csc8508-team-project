set(SOURCES
    BulletDebug.cpp
    BulletDebug.h
    GameTechRenderer.cpp
    GameTechRenderer.h
    main.cpp
    NetworkedGame.cpp
    NetworkedGame.h
    NetworkPlayer.cpp
    NetworkPlayer.h
    StateGameObject.cpp
    StateGameObject.h
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
)

set(COMPILED_MESHES
    Capsule.msh
    Plane.msh
    Cube.msh
    Sphere.msh
    ORIGAMI_Chat.msh
    Kitten.msh
    Keeper.msh
    19463_Kitten_Head_v1.msh
    /Max/Rig_Maximilian.msh
    /MaleGuard/Male_Guard.msh
    /FemaleGuard/Female_Guard.msh
)

add_executable(${PROJECT_NAME} ${SOURCES})
add_dependencies(${PROJECT_NAME} MeshCompiler)

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
    ${CMAKE_SOURCE_DIR}/TeamProject/
    ${CMAKE_SOURCE_DIR}/Network/
)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRECTORIES}>)

target_link_libraries(${PROJECT_NAME} NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} CSC8503CoreClasses)
target_link_libraries(${PROJECT_NAME} OpenGLRendering)
target_link_libraries(${PROJECT_NAME} NetworkLib)
target_link_libraries(${PROJECT_NAME} ${BULLET_LIBRARIES})
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

foreach(file ${COMPILED_MESHES})
    set(out "${CMAKE_SOURCE_DIR}/Assets/Meshes/${file}b")
    add_custom_command(
        OUTPUT ${out}
        COMMAND MeshCompiler ${file} ${out}
        DEPENDS "${CMAKE_SOURCE_DIR}/Assets/Meshes/${file}" MeshCompiler
        COMMENT "Compiling mesh ${file}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    list(APPEND PROCESSED_MESHES ${out})
endforeach()
add_custom_target(ProcessMeshes ALL DEPENDS ${PROCESSED_MESHES})
add_dependencies(${PROJECT_NAME} ProcessMeshes)
