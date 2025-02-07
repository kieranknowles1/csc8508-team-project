set(SOURCES
    main.cpp
)

add_executable(MeshCompiler ${SOURCES})
target_link_libraries(MeshCompiler NCLCoreClasses)
target_include_directories(MeshCompiler PRIVATE ${CMAKE_SOURCE_DIR})
set_target_properties(MeshCompiler PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
