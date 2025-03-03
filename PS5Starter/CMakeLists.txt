set(PROJECT_NAME PS5Starter)

################################################################################
# Source groups
################################################################################
set(SOURCES
    GameTechAGCRenderer.h
    GameTechAGCRenderer.cpp
    main.cpp
)

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

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIRS})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <memory>
    <unordered_set>
    <vector>
    <map>
    <set>

    <string>
    <fstream>
    <iostream>
    <iosfwd>

    <Matrix.h>
    <Vector.h>

	<Plane.h>
	<Frustum.h>

    <Camera.h>
    <GameTimer.h>
    <TextureLoader.h>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
	VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)"
)
################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
    )
endif()

################################################################################
# Dependencies
################################################################################
target_include_directories (${PROJECT_NAME}
    PUBLIC ${CMAKE_SOURCE_DIR}/NCLCoreClasses
    PUBLIC ${CMAKE_SOURCE_DIR}/PS5Core

	PUBLIC ${CMAKE_SOURCE_DIR}/CSC8503CoreClasses
	PUBLIC ${CMAKE_SOURCE_DIR}/TeamProject
)

# Set FileType to Document for all files using the WavePsslc tool
set_property(TARGET PS5Starter PROPERTY VS_SOURCE_SETTINGS_WavePsslc "FileType=Document")

set(PROJECT_DEPENDENCIES
    ${BULLET_LIBRARIES}
    NCLCoreClasses
    PS5Core
    CSC8508
    CSC8503CoreClasses
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE ${PROJECT_DEPENDENCIES}
    PRIVATE SceAgcDriver_stub_weak
    PRIVATE SceAgc_stub_weak
    PRIVATE SceVideoOut_stub_weak
    PRIVATE SceAgc_debug_nosubmission
    PRIVATE SceAgcCore_debug_nosubmission
    PRIVATE SceAgcGpuAddress_debug_nosubmission

	PRIVATE ScePad_stub_weak
	PRIVATE SceUserService_stub_weak
    PRIVATE SceNpUtility_stub_weak
    PRIVATE SceCommonDialog_stub_weak
    PRIVATE SceNetCtlApDialog_stub_weak
    PRIVATE SceNetCtlAp_stub_weak
    PRIVATE SceNetCtl_stub_weak
    PRIVATE SceNet_stub_weak
    PRIVATE ScePosix_stub_weak
)

include_directories(${COMPILE_ASSET_ROOT})

foreach(file ${SCE_MODULES})
    file(COPY ${PS5_SCE_MODULE}/${file} DESTINATION ${CMAKE_BINARY_DIR}/sce_module/)
endforeach()
