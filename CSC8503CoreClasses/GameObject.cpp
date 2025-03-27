#include "GameObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "WorldState.h"

#include "LinearMath/btQuaternion.h"

using namespace NCL::CSC8503;
using namespace WorldState;

GameObject::GameObject(const std::string& objectName)	{
	name			= objectName;
	worldID			= -1;
	isActive		= true;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	networkObject	= nullptr;

}

GameObject::~GameObject()	{
	delete physicsObject;
	delete renderObject;
	delete networkObject;

    if (owner) delete owner;

	if (objects.contains(worldID)) objects.erase(worldID);
}

