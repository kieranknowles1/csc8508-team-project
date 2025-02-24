set(PROJECT_NAME OpenGLRendering)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "OGLTexture.h"
    "OGLShader.h"
    "OGLRenderer.h"
    "OGLMesh.h"
    "OGLComputeShader.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "OGLTexture.cpp"
    "OGLShader.cpp"
    "OGLRenderer.cpp"
    "OGLMesh.cpp"
    "OGLComputeShader.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(GLAD
    "glad/gl.h"
    "glad/gl.c"
)
source_group("GLAD" FILES ${GLAD})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    ${GLAD}
)

set_source_files_properties(${ALL_FILES} PROPERTIES LANGUAGE CXX)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

#use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

set(ROOT_NAMESPACE OpenGLRendering)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <chrono>
    <memory>
    <unordered_set>
    <vector>
    <string>
    <fstream>
    <iostream>
    <string>
    <iosfwd>
    <set>
	<assert.h>
    <functional>

    <NCLCoreClasses/Vector.h>
    <NCLCoreClasses/Matrix.h>
)

target_include_directories (${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/NCLCoreClasses
    PRIVATE ${CMAKE_SOURCE_DIR}/OpenGLRendering
)
################################################################################
# Dependencies
################################################################################
find_package(OpenGL REQUIRED)
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "NCLCoreClasses"
    ${OPENGL_LIBRARIES}
)

if(USE_SDL2)
    find_package(SDL2 REQUIRED)
    list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES SDL2::SDL2)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
