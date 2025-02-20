set(PROJECT_NAME PS5Starter)

################################################################################
# Source groups
################################################################################
set(SOURCES
    GameTechAGCRenderer.h
    GameTechAGCRenderer.cpp
    main.cpp
)
source_group("Shader Files" FILES ${SHADERS})

################################################################################
# Target
################################################################################
# TODO: Shaders aren't compiling properly when made their own target. How do we access wavepsslc?
add_executable(${PROJECT_NAME} ${SOURCES} ${SHADERS})
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

# Set the tool and GenerateHeader on PSSL files in ${PSSL_SHADERS}
set_property(SOURCE ${SHADERS} PROPERTY VS_TOOL_OVERRIDE WavePsslc)
set_property(SOURCE ${SHADERS} PROPERTY VS_SETTINGS "GenerateHeader=false")
set_property(SOURCE ${SHADERS} PROPERTY VS_SETTINGS "OutputFileName=${COMPILE_ASSET_ROOT}Shaders/PSSL/%(FileName).ags")
set_property(SOURCE ${SHADERS} PROPERTY OUTPUT_NAME "$(Test)%(FileName).ags")


# Set FileType to Document for all files using the WavePsslc tool
set_property(TARGET PS5Starter PROPERTY VS_SOURCE_SETTINGS_WavePsslc "FileType=Document")

foreach (file ${TEXTURES})
    get_filename_component(file_name ${file} NAME)
	get_filename_component(file_ext ${file} EXT)

	get_filename_component(file_dir ${file} ABSOLUTE)
	#set(SPIRV_OUTPUT ${file_name}.spv)
	set(TEX_INPUT ${file_dir})
	set(TEX_OUTPUT ${file_dir}.gnf)

	add_custom_command(
		OUTPUT ${TEX_OUTPUT}

		COMMENT "Generating GNF for texture ${file}"
		COMMAND image2gnf.exe -g 1 -i "${file}" -o ${TEX_OUTPUT} -f Bc1UNorm
		DEPENDS ${file}
		VERBATIM
	)
	list(APPEND TEX_GNF_FILES ${TEX_OUTPUT})
endforeach()

add_custom_target(
    Textures
	DEPENDS ON ${TEX_GNF_FILES}
)
set(PROJECT_DEPENDENCIES
    ${BULLET_LIBRARIES}
    NCLCoreClasses
    PS5Core
    CSC8508
    CSC8503CoreClasses
)

add_dependencies(${PROJECT_NAME} Textures)

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
)

include_directories(${COMPILE_ASSET_ROOT})
