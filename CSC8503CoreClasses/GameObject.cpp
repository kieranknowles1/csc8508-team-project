#include "GameObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "../TeamProject/Multiplayer/WorldState.hpp"

using namespace NCL::CSC8503;

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

	if (objects.contains(worldID)) objects.erase(worldID);
}

void GameObject::UpdateObjectState() {
    std::vector<std::pair<WorldState::StateType, WorldState::StateValue>> stateUpdates;
    stateUpdates.push_back(
        { WorldState::StateType::LinearVelocity, GetPhysicsObject()->GetRigidBody()->getLinearVelocity() }
    );
    stateUpdates.push_back(
        { WorldState::StateType::AngularVelocity, GetPhysicsObject()->GetRigidBody()->getAngularVelocity() }
    );
    stateUpdates.push_back(
        { WorldState::StateType::Position, GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin() }
    );
    stateUpdates.push_back(
        { WorldState::StateType::Rotation, GetPhysicsObject()->GetRigidBody()->getWorldTransform().getRotation() }
    );
    objectWorldState.UpdateStates(stateUpdates);
}

void GameObject::UpdateFromState() {
    if (objectWorldState.HasState()) {
        btRigidBody* body = GetPhysicsObject()->GetRigidBody();

        objectWorldState.AcquireReadLock();

        body->setLinearVelocity(std::get<btVector3>(objectWorldState.UnsafeReadState(WorldState::StateType::LinearVelocity)));
        body->setAngularVelocity(std::get<btVector3>(objectWorldState.UnsafeReadState(WorldState::StateType::AngularVelocity)));

        body->getWorldTransform().setOrigin(std::get<btVector3>(objectWorldState.UnsafeReadState(WorldState::StateType::Position)));
        body->getWorldTransform().setRotation(std::get<btQuaternion>(objectWorldState.UnsafeReadState(WorldState::StateType::Rotation)));

        objectWorldState.ReleaseReadLock();
        objectWorldState.Clear();
    }
}
