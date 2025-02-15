set(PROJECT_NAME CSC8503CoreClasses)

################################################################################
# Source groups
################################################################################
set(AI_Behaviour_Tree
    "BehaviourAction.h"
    "BehaviourNode.h"
    "BehaviourNodeWithChildren.h"
    "BehaviourSelector.h"
    "BehaviourSelector.cpp"
    "BehaviourSequence.h"
    "BehaviourSequence.cpp"
)
source_group("AI\\Behaviour Trees" FILES ${AI_Behaviour_Tree})

set(AI_Pushdown_Automata
    "PushdownMachine.h"
    "PushdownMachine.cpp"
    "PushdownState.h"
    "PushdownState.cpp"
)
source_group("AI\\Pushdown Automata" FILES ${AI_Pushdown_Automata})

set(AI_State_Machine
    "State.h"
    "StateMachine.h"
    "StateMachine.cpp"
    "StateMachine.h"
    "StateTransition.h"
)
source_group("AI\\State Machine" FILES ${AI_State_Machine})

set(AI_Pathfinding
    "NavigationGrid.h"
    "NavigationGrid.cpp"
    "NavigationMesh.cpp"
    "NavigationMesh.h"
    "NavigationMap.h"
    "NavigationPath.h"
)
source_group("AI\\Pathfinding" FILES ${AI_Pathfinding})


set(Collision_Detection
    "Ray.h"
)
source_group("Collision Detection" FILES ${Collision_Detection})

set(Networking
    "GameClient.h"
    "GameClient.cpp"
    "GameServer.h"
    "GameServer.cpp"
    "NetworkBase.h"
    "NetworkBase.cpp"
    "NetworkObject.h"
    "NetworkObject.cpp"
    "NetworkState.h"
    "NetworkState.cpp"
)
source_group("Networking" FILES ${Networking})

set(Physics
    "PhysicsObject.cpp"
    "PhysicsObject.h"
    "CustomCollisionCallback.h"
    "CollisionInfo.cpp"
    "CollisionInfo.h"
)
source_group("Physics" FILES ${Physics})

set(Header_Files
    "Debug.h"
    "GameObject.h"
    "GameWorld.h"
    "RenderObject.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "Debug.cpp"
    "GameObject.cpp"
    "GameWorld.cpp"
    "RenderObject.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(enet_Files
    "./enet/callbacks.c"
    # "./enet/callbacks.h"
    "./enet/compress.c"
    "./enet/enet.h"
    "./enet/host.c"
    "./enet/list.c"
    "./enet/list.h"
    "./enet/packet.c"
    "./enet/peer.c"
    "./enet/protocol.c"
    "./enet/protocol.h"
    "./enet/time.h"
    "./enet/types.h"
    "./enet/utility.h"
)
if(WIN32)
  list(APPEND enet_Files "./enet/win32.h" "./enet/win32.c")
else()
  list(APPEND enet_Files "./enet/unix.h" "./enet/unix.c")
endif()

source_group("eNet" FILES ${enet_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    ${AI_Behaviour_Tree}
    ${AI_Pushdown_Automata}
    ${AI_State_Machine}
    ${AI_Pathfinding}
    ${Collision_Detection}
    ${Networking}
    ${Physics}
    ${enet_Files}
)

set_source_files_properties(${ALL_FILES} PROPERTIES LANGUAGE CXX)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${BULLET_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIRS})

#set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS c;C;M;c++;cc;cpp;cxx;m;mm;mpp;CPP;ixx;cppm)

# use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE CSC8503CoreClasses)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "UNICODE"
    "_UNICODE"
    "WIN32_LEAN_AND_MEAN"
    "_WINSOCK_DEPRECATED_NO_WARNINGS"
	"NOMINMAX"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vector>
    <map>
    <stack>
    <string>
    <list>
    <thread>
    <atomic>
    <functional>
    <iostream>
    <set>
    "../NCLCoreClasses/Vector.h"
    "../NCLCoreClasses/Plane.h"
    "../NCLCoreClasses/Matrix.h"
	"../NCLCoreClasses/GameTimer.h"
)

################################################################################
# Compile and link options
################################################################################


################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "NCLCoreClasses"
    "ws2_32.lib"
)

include_directories("../NCLCoreClasses/")
include_directories("./")

if(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE "ws2_32.lib")
endif()
