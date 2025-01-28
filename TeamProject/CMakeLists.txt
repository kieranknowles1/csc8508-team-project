set(SOURCES
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} OpenGLRendering)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
