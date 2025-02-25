set(TEXTURE_EXTENSIONS ".jpg" ".png" ".dds")

macro(process_file)
    set(outvar ${ARGV0})
    set(file ${ARGV1})
    get_filename_component(extension ${file} EXT)
    string(TOLOWER ${extension} extension)


    if(${extension} STREQUAL ".msh")
        set(${outvar} ${file}b)
        list(APPEND PROCESSED_ASSETS ${file}b)
        add_custom_command(
            OUTPUT ${file}b
            COMMAND MeshCompiler ${file} ${file}b
            DEPENDS ${file}
        )
    elseif(${extension} IN_LIST TEXTURE_EXTENSIONS AND PS5_BUILD)
        set(${outvar} ${file}.gnf)
        list(APPEND PROCESSED_ASSETS ${file}.gnf)
        add_custom_command(
            OUTPUT ${file}.gnf
            COMMAND image2gnf -g 1 -i ${file} -o ${file}.gnf -f Auto
            DEPENDS ${file}
        )
    elseif(${extension} STREQUAL ".pssl" AND PS5_BUILD)
        string(REGEX REPLACE "\\.pssl$" ".ags" ${outvar} ${file})
        list(APPEND PROCESSED_ASSETS ${${outvar}})
        add_custom_command(
            OUTPUT ${${outvar}}
            COMMAND "${PS5_SDK_TOOLS}/prospero-wave-psslc" ${file} -o ${${outvar}}
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
    if(PS5_BUILD)
        message("Shaders will be built to ${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    add_dependencies(${afterTarget} MeshCompiler)

    foreach(asset ${args})
        process_file(processed ${asset})
        file(RELATIVE_PATH relative ${assetRoot} ${processed})

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
