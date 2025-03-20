#include "LaserObject.h"
#include "Multiplayer/WorldState.hpp"
#include "Multiplayer/GamePackets.hpp"

using namespace WorldState;
using namespace NCL::CSC8503;

void LaserObject::UpdateObjectState() {
    StateReader writeReader = states->GetWriteState();
    ObjectState* writeState = writeReader.GetState();
    
    btRigidBody* body = GetPhysicsObject()->GetRigidBody();
    btTransform transform = body->getWorldTransform();

    writeState->Lock();

    writeState->UpdateState(StateType::StartPos, startPos);
    writeState->UpdateState(StateType::EndPos, endPos);
    writeState->UpdateState(StateType::Normal, collisionNormal);

    writeState->Unlock();
}

void LaserObject::UpdateFromState(float dt) {
    elapsedTickTime += dt;

    std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };
    float weight = TICK_UPDATE_RATE / fmod(elapsedTickTime, TICK_UPDATE_RATE);

    StateReader currentReader = states->GetCurrentState();
    StateReader readReader = states->GetReadState();

    ObjectState* current = currentReader.GetState();
    ObjectState* read = currentReader.GetState();

    btRigidBody* body = GetPhysicsObject()->GetRigidBody();

    StateValue currentStartPosValue;
    StateValue targetStartPosValue;

    StateValue currentEndPosValue;
    StateValue targetEndPosValue;
    
    StateValue currentCollisionNormalValue;
    StateValue targetCollisionNormalValue;
    
    // Reading.
    current->Lock_Shared();
    read->Lock_Shared();

    bool hasCurrentStartPos = current->ReadState(StateType::StartPos, &currentStartPosValue);
    bool hasTargetStartPos = read->ReadState(StateType::StartPos, &targetStartPosValue);

    bool hasCurrentEndPos = current->ReadState(StateType::EndPos, &currentEndPosValue);
    bool hasTargetEndPos = read->ReadState(StateType::EndPos, &targetEndPosValue);

    bool hasCurrentCollisionNormal = current->ReadState(StateType::Normal, &currentCollisionNormalValue);
    bool hasTargetCollisionNormal = read->ReadState(StateType::Normal, &targetCollisionNormalValue);

    current->Unlock_Shared();
    read->Unlock_Shared();

    currentReader.Unlock();
    readReader.Unlock();

    // Start Position.
    if (hasCurrentStartPos && hasCurrentEndPos) {
        btVector3 currentStartPos = std::get<btVector3>(currentStartPosValue);
        btVector3 targetStartPos = std::get<btVector3>(currentEndPosValue);
        btVector3 interpolated = btVector3(
            lerp(currentStartPos.x(), targetStartPos.x(), weight),
            lerp(currentStartPos.y(), targetStartPos.y(), weight),
            lerp(currentStartPos.z(), targetStartPos.z(), weight)
        );
        startPos = interpolated;
    } 

    // End Position.
    if (hasCurrentEndPos && hasCurrentEndPos) {
        btVector3 currentEndPos = std::get<btVector3>(currentEndPosValue);
        btVector3 targetEndPos = std::get<btVector3>(currentEndPosValue);
        btVector3 interpolated = btVector3(
            lerp(currentEndPos.x(), targetEndPos.x(), weight),
            lerp(currentEndPos.y(), targetEndPos.y(), weight),
            lerp(currentEndPos.z(), targetEndPos.z(), weight)
        );
        endPos = interpolated;
    } 

    // Collision Normal.
    if (hasCurrentCollisionNormal && hasCurrentEndPos) {
        btVector3 currentCollisionNormal = std::get<btVector3>(currentCollisionNormalValue);
        btVector3 targetCollisionNormal = std::get<btVector3>(currentEndPosValue);
        btVector3 interpolated = btVector3(
            lerp(currentCollisionNormal.x(), targetCollisionNormal.x(), weight),
            lerp(currentCollisionNormal.y(), targetCollisionNormal.y(), weight),
            lerp(currentCollisionNormal.z(), targetCollisionNormal.z(), weight)
        );
        collisionNormal = interpolated;
    } 
}

std::vector<std::shared_ptr<Packet::Packet>> LaserObject::CreatePackets(int sequenceNum) {
    std::vector<std::shared_ptr<Packet::Packet>> packets;

    StateReader readReader = GetObjectStates()->GetReadState();
    ObjectState* read = readReader.GetState();

    StateValue startPosValue;
    StateValue endPosValue;
    StateValue collisionNormalValue;

    read->Lock_Shared();
    
    bool hasStartPos = read->ReadState(StateType::StartPos, &startPosValue);
    bool hasEndPos = read->ReadState(StateType::EndPos, &endPosValue);
    bool hasCollisionNormal = read->ReadState(StateType::Normal, &collisionNormalValue);

    read->Unlock_Shared();
    readReader.Unlock();
    
    if (hasStartPos && hasEndPos && hasCollisionNormal) {
        packets.push_back(std::move(std::make_shared<Packet::LaserPacket>(
            GetOwner()->GetUserID(),
            std::get<btVector3>(startPosValue),
            std::get<btVector3>(endPosValue),
            std::get<btVector3>(collisionNormalValue),
            sequenceNum
        )));
    }

    return packets;
}
