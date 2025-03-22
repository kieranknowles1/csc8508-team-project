#include "GameObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "WorldState.h"

#include "../TeamProject/Multiplayer/GamePackets.hpp"
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

    states = std::make_unique<StateBuffer>();
}

GameObject::~GameObject()	{
	delete physicsObject;
	delete renderObject;
	delete networkObject;

    if (owner) delete owner;

	if (objects.contains(worldID)) objects.erase(worldID);
}

void GameObject::UpdateWorldState() {
    auto [writeState, lock] = GetWorldStates()->GetWriteState();
    
    btRigidBody* body = GetPhysicsObject()->GetRigidBody();
    btTransform transform = body->getWorldTransform();

    std::unique_lock stateLock(writeState->Lock());

    writeState->UpdateState(StateType::LinearVelocity, body->getLinearVelocity());
    writeState->UpdateState(StateType::AngularVelocity, body->getAngularVelocity());
    writeState->UpdateState(StateType::Position, transform.getOrigin());
    writeState->UpdateState(StateType::Rotation, transform.getRotation());
}

void GameObject::UpdateFromWorldState(float dt) {
    elapsedTickTime += dt;

    std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };
    float weight = fmod(elapsedTickTime, TICK_UPDATE_RATE) / TICK_UPDATE_RATE;

    auto [current, currentLock] = GetWorldStates()->GetCurrentState();
    auto [read, readLock] = GetWorldStates()->GetReadState();

    btRigidBody* body = GetPhysicsObject()->GetRigidBody();

    StateValue currentLinearValue;
    StateValue targetLinearValue;

    StateValue currentAngularValue;
    StateValue targetAngularValue;
    
    StateValue currentPositionValue;
    StateValue targetPositionValue;
    
    StateValue currentRotationValue;
    StateValue targetRotationValue;

    // Reading.
    std::shared_lock currentStateLock = current->Lock_Shared();
    std::shared_lock readStateLock = read->Lock_Shared();

    bool hasCurrentLinear = current->ReadState(StateType::LinearVelocity, &currentLinearValue);
    bool hasTargetLinear = read->ReadState(StateType::LinearVelocity, &targetLinearValue);

    bool hasCurrentAngular = current->ReadState(StateType::AngularVelocity, &currentAngularValue);
    bool hasTargetAngular = read->ReadState(StateType::AngularVelocity, &targetAngularValue);

    bool hasCurrentPosition = current->ReadState(StateType::Position, &currentPositionValue);
    bool hasTargetPosition = read->ReadState(StateType::Position, &targetPositionValue);

    bool hasCurrentRotation = current->ReadState(StateType::Rotation, &currentRotationValue);
    bool hasTargetRotation = read->ReadState(StateType::Rotation, &targetRotationValue);

    currentStateLock.unlock();
    readStateLock.unlock();

    currentLock.unlock();
    readLock.unlock();

    // Linear Velocity.
    if (hasCurrentLinear && hasTargetLinear) {
        btVector3 currentLinear = std::get<btVector3>(currentLinearValue);
        btVector3 targetLinear = std::get<btVector3>(targetLinearValue);
        btVector3 interpolated = btVector3(
            lerp(currentLinear.x(), targetLinear.x(), weight),
            lerp(currentLinear.y(), targetLinear.y(), weight),
            lerp(currentLinear.z(), targetLinear.z(), weight)
        );
        body->setLinearVelocity(interpolated);
    } 

    // Angular Velocity.
    if (hasCurrentAngular && hasTargetAngular) {
        btVector3 currentAngular = std::get<btVector3>(currentAngularValue);
        btVector3 targetAngular = std::get<btVector3>(targetAngularValue);
        btVector3 interpolated = btVector3(
            lerp(currentAngular.x(), targetAngular.x(), weight),
            lerp(currentAngular.y(), targetAngular.y(), weight),
            lerp(currentAngular.z(), targetAngular.z(), weight)
        );
        body->setAngularVelocity(interpolated);
    }

    // Position.
    if (hasCurrentPosition && hasTargetPosition) {
        btVector3 currentPosition = std::get<btVector3>(currentPositionValue);
        btVector3 targetPosition = std::get<btVector3>(targetPositionValue);
        btVector3 interpolated = btVector3(
            lerp(currentPosition.x(), targetPosition.x(), weight),
            lerp(currentPosition.y(), targetPosition.y(), weight),
            lerp(currentPosition.z(), targetPosition.z(), weight)
        );
        body->getWorldTransform().setOrigin(interpolated);
    } 

    // Rotation.
    if (hasCurrentRotation && hasTargetRotation) {
        btQuaternion currentRotation = std::get<btQuaternion>(currentRotationValue);
        btQuaternion targetRotation = std::get<btQuaternion>(targetRotationValue);
        btQuaternion interpolated = btQuaternion(
            lerp(currentRotation.x(), targetRotation.x(), weight),
            lerp(currentRotation.y(), targetRotation.y(), weight),
            lerp(currentRotation.z(), targetRotation.z(), weight),
            lerp(currentRotation.w(), targetRotation.w(), weight)
        );
        body->getWorldTransform().setRotation(interpolated);
    }
}

std::vector<std::shared_ptr<Packet::Packet>> GameObject::CreatePackets(int sequenceNum) {
    std::vector<std::shared_ptr<Packet::Packet>> packets;

    auto[read, readLock] = GetWorldStates()->GetReadState();

    StateValue linearValue;
    StateValue angularValue;
    StateValue positionValue;
    StateValue rotationValue;

    std::shared_lock readStateLock = read->Lock_Shared();

    bool hasLinear = read->ReadState(StateType::LinearVelocity, &linearValue);
    bool hasAngular = read->ReadState(StateType::AngularVelocity, &angularValue);

    bool hasPosition = read->ReadState(StateType::Position, &positionValue);
    bool hasRotation = read->ReadState(StateType::Rotation, &rotationValue);

    readStateLock.unlock();
    readLock.unlock();
    
    if (hasLinear && hasAngular) {
        packets.push_back(std::move(std::make_shared<Packet::DeltaPacket>(
            GetWorldID(),
            std::get<btVector3>(linearValue),
            std::get<btVector3>(angularValue),
            sequenceNum
        )));
    }

    if (hasPosition && hasRotation) {
        packets.push_back(std::move(std::make_shared<Packet::PositionPacket>(
            GetWorldID(),
            std::get<btVector3>(positionValue),
            std::get<btQuaternion>(rotationValue),
            sequenceNum
        )));
    }
    return std::move(packets);
}
