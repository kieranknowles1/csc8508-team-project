macro(fmod_copy_libs)
    # Get the target to copy the dlls next to
    if (${ARGV0})
        set (TARGET ${ARGV0})
    else()
        set (TARGET ${PROJECT_NAME})
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

    foreach(dll ${FMOD_DLLS})
        message("Copying DLL ${dll}")
        get_filename_component(FMOD_DLL_FILENAME ${dll} NAME)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${dll}
                ${TARGET_DIR}/${FMOD_DLL_FILENAME}
        )
    endforeach()
endmacro()
