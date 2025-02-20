set(SOURCES
    BulletDebug.cpp
    BulletDebug.h
    GameTechRenderer.cpp
    GameTechRenderer.h
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
)

if(NOT PS5_BUILD)
    list(APPEND SOURCES main.cpp)
    add_executable(${PROJECT_NAME} ${SOURCES})
else()
    # There is quite a bit of extra work for PS5 builds,
    # so this is split into PS5Starter where we link against
    # the main game statically
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()

# TODO: PS5 Support
if(NOT PS5_BUILD)
    add_dependencies(${PROJECT_NAME} MeshCompiler)
endif()

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

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIRS})

foreach(file ${MESHES})
    set(out "${file}b")
    add_custom_command(
        OUTPUT ${out}
        COMMAND MeshCompiler ${file} ${out}
        DEPENDS ${file} MeshCompiler
        COMMENT "Compiling mesh ${file}"
    )
    list(APPEND PROCESSED_MESHES ${out})
endforeach()
add_custom_target(ProcessMeshes ALL DEPENDS ${PROCESSED_MESHES})

set(RUNTIME_ASSETS
    ${PROCESSED_MESHES}
    ${SHADERS}
    ${TEXTURES}
    ${LEVELS}
    ${DATA}
    ${FONTS}
)

message("Will copy assets to ${CMAKE_CURRENT_BINARY_DIR}")
foreach(file ${RUNTIME_ASSETS})
    file(RELATIVE_PATH relative ${CMAKE_SOURCE_DIR} ${file})
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${file}
            "${CMAKE_CURRENT_BINARY_DIR}/${relative}"
    )
endforeach()


# TODO: PS5 support
if(NOT PS5_BUILD)
    add_dependencies(${PROJECT_NAME} ProcessMeshes)
endif()
