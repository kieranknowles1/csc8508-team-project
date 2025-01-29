set(CMAKE_CONFIGURATION_TYPES "Debug;Release;NetworkTest" CACHE STRING "Build Configurations" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_NETWORKTEST "" CACHE STRING "Linker flags for NetworkTest Configuation")

set(SOURCES
	main.cpp
	network.cpp
)

set(NETWORK_TEST_SOURCES
	network_test.cpp
	network.cpp
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
	$<$<CONFIG:Debug>:${SOURCES}>
	$<$<CONFIG:Release>:${SOURCES}>
	$<$<CONFIG:NetworkTest>:${NETWORK_TEST_SOURCES}>
)

target_link_libraries(${PROJECT_NAME} NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} OpenGLRendering)


# Correct way to define NETWORK_TEST for Visual Studio
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:NetworkTest>:NETWORK_TEST>
)

# Correct way to set compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:NetworkTest>:-O2>
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
