set(SOURCES
	Network.cpp
)

set(TEST_SOURCES
	NetworkTest.cpp
)

add_library(Network ${SOURCES})
add_executable(NetworkTest ${SOURCES} ${TEST_SOURCES})

set(INCLUDE_DIRECTORIES
	${CMAKE_SOURCE_DIR}/NCLCoreClasses/
	${CMAKE_SOURCE_DIR}/CSC8503CoreClasses/
)

include_directories(${INCLUDE_DIRECTORIES} PRIVATE)

target_include_directories(Network PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(Network
	NCLCoreClasses
	CSC8503CoreClasses
)

target_compile_definitions(Network PRIVATE $<$<CONFIG:Debug>:NETWORK_TEST>)