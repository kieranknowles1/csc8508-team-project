macro(process_file)
    set(outvar ${ARGV0})
    set(file ${ARGV1})
    get_filename_component(extension ${file} EXT)
    string(TOLOWER ${extension} extension)


    if(extension STREQUAL ".msh")
        set(${outvar} ${file}b)
        list(APPEND PROCESSED_ASSETS ${file}b)
        add_custom_command(
            OUTPUT ${file}b
            COMMAND MeshCompiler ${file} ${file}b
            DEPENDS ${file} MeshCompiler
        )
    else()
        set(${outvar} ${file})
    endif()
endmacro()

# copy_assets(destination, assetRoot, afterTarget, assets...)
# Copy assets, preprocessing them depending on filetype
macro(copy_assets)
    set(args ${ARGN})
    list(POP_FRONT args destination assetRoot afterTarget)

    add_dependencies(${afterTarget} MeshCompiler)

    foreach(asset ${args})
        process_file(processed ${asset})
        file(RELATIVE_PATH relative ${assetRoot} ${processed})
        message("Will copy ${processed} to ${relative}")

        add_custom_command(
            TARGET ${afterTarget} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${processed}
            ${destination}/${relative}
        )
    endforeach()

    add_custom_target(${afterTarget}-ProcessAssets ALL DEPENDS ${PROCESSED_ASSETS})
    add_dependencies(${afterTarget} ${afterTarget}-ProcessAssets)
endmacro()
