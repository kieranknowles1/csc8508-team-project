# On Windows platform, copies the appropriate DLLs next to the target
# Okay to call on other platforms, but logs a warning
# @param TARGET (optional: default is the current project name)
# @note depends on FMOD_DLL and FMOD_STUDIO_DLL being cached in the fmod/CMakeLists.txt file
macro(fmod_copy_dlls)
    if (NOT WIN32 AND NOT PS5_BUILD)
            message(WARNING "fmod-cmake: A call to `fmod_copy_dlls` was made on a non-Windows platform")
        return()
    endif()

    # Get the target to copy the dlls next to
    if (${ARGV0})
        set (TARGET ${ARGV0})
    else()
        set (TARGET ${PROJECT_NAME})
    endif()

    # Copy FMOd Core dll if user provided one in the fmod directory
    if(NOT PS5_BUILD)
        get_target_property(TARGET_DIR ${TARGET} BINARY_DIR)
    else()
        set(TARGET_DIR ${CMAKE_BINARY_DIR})
    endif()

    if (NOT EXISTS ${TARGET_DIR})
        message(WARNING "Could not copy FMOD Core dll: Failed to find target directory for target: ${TARGET}.")
        return()
    endif()

    foreach(dll ${FMOD_DLL} ${FMOD_STUDIO_DLL})
        get_filename_component(FMOD_DLL_FILENAME ${dll} NAME)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${dll}
                ${TARGET_DIR}/${FMOD_DLL_FILENAME}
        )
    endforeach()
endmacro()

macro(fmod_copy_libs)
    # Get the target to copy the dlls next to
    if (${ARGV0})
        set (TARGET ${ARGV0})
    else()
        set (TARGET ${PROJECT_NAME})
    endif()

    if (WIN32 OR PS5_BUILD)
        fmod_copy_dlls(${TARGET})
    endif()

    get_target_property(TARGET_DIR ${TARGET} BINARY_DIR)

    if (NOT EXISTS ${TARGET_DIR})
        message(WARNING "Could not copy FMOD Core dll: Failed to find target directory for target: ${TARGET}.")
        return()
    endif()

    foreach(LIB ${FMOD_LIBS} ${FMOD_STUDIO_LIBS})
        get_filename_component(LIB_NAME ${LIB} NAME)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${LIB}
                ${TARGET_DIR}/${LIB_NAME}
        )
    endforeach()
endmacro()
