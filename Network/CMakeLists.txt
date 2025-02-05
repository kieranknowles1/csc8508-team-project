set(SOURCES
	Server.cpp
	Server.hpp
	Client.cpp
	Client.hpp
	Network.hpp
)

set(TEST_SOURCES
	TestMain.cpp
)

set(INCLUDE_DIRECTORIES
	${CMAKE_SOURCE_DIR}/NCLCoreClasses/
	${CMAKE_SOURCE_DIR}/CSC8503CoreClasses/
)

# Assign a folder to a target.
function(assign_folder target folder_name)
	set_target_properties(${target} PROPERTIES FOLDER ${folder_name})
endfunction()

# Production Network Library
add_library(NetworkLib ${SOURCES})
target_include_directories(NetworkLib PUBLIC ${INCLUDE_DIRECTORIES})
target_link_libraries(NetworkLib PUBLIC
	NCLCoreClasses
	CSC8503CoreClasses
)
assign_folder(NetworkLib "Network")

# Testing library builds with NETWORK_TEST compiler definition.
add_library(NetworkTestLib ${SOURCES})
target_include_directories(NetworkTestLib PUBLIC ${INCLUDE_DIRECTORIES})
target_link_libraries(NetworkTestLib PUBLIC
	NCLCoreClasses
	CSC8503CoreClasses
)
assign_folder(NetworkTestLib "Network/Test")
target_compile_definitions(NetworkTestLib PRIVATE NETWORK_TEST)

# Add test executable.
add_executable(NetworkTest ${TEST_SOURCES})
target_include_directories(NetworkTest PUBLIC ${INCLUDE_DIRECTORIES})
target_link_libraries(NetworkTest PUBLIC
	NetworkTestLib
)
assign_folder(NetworkTest "Network/Test")

# Enet Linking.
if(MSVC)
    target_link_libraries(NetworkLib PUBLIC "Winmm.lib")
    target_link_libraries(NetworkTestLib PUBLIC "Winmm.lib")
endif()
