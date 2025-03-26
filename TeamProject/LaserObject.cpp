#include "LaserObject.h"
#include "WorldState.h"
#include "Multiplayer/GamePackets.hpp"
#include "Shoot.h"


using namespace WorldState;
using namespace NCL::CSC8503;


void LaserObject::Update(float dt) {
    // Spawning a decal at the end position.
    if (endPos != startPos) {
        Shoot::GetInstance()->SpawnDecal(endPos, collisionNormal, color);
    }
}


void LaserObject::UpdateWorldState() {
    auto [writeState, lock] = GetWorldStates()->GetWriteState();
    
    std::unique_lock stateLock = writeState->Lock();
    writeState->UpdateState(StateType::StartPos, startPos);
    writeState->UpdateState(StateType::EndPos, endPos);
    writeState->UpdateState(StateType::Normal, collisionNormal);
}

void LaserObject::UpdateFromWorldState(float dt) {
    elapsedTickTime += dt;

    std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };
    float weight = TICK_UPDATE_RATE / fmod(elapsedTickTime, TICK_UPDATE_RATE);

    auto [current, currentLock] = GetWorldStates()->GetCurrentState();
    auto [read, readLock] = GetWorldStates()->GetReadState();

    StateValue currentStartPosValue;
    StateValue targetStartPosValue;

    StateValue currentEndPosValue;
    StateValue targetEndPosValue;
    
    StateValue currentCollisionNormalValue;
    StateValue targetCollisionNormalValue;
    
    // Reading.
    std::shared_lock currentStateLock = current->Lock_Shared();
    std::shared_lock readStateLock = read->Lock_Shared();

    bool hasCurrentStartPos = current->ReadState(StateType::StartPos, &currentStartPosValue);
    bool hasTargetStartPos = read->ReadState(StateType::StartPos, &targetStartPosValue);

    bool hasCurrentEndPos = current->ReadState(StateType::EndPos, &currentEndPosValue);
    bool hasTargetEndPos = read->ReadState(StateType::EndPos, &targetEndPosValue);

    bool hasTargetCollisionNormal = read->ReadState(StateType::Normal, &targetCollisionNormalValue);

    currentStateLock.unlock();
    readStateLock.unlock();

    currentLock.unlock();
    readLock.unlock();

    // Start Position.
    if (hasCurrentStartPos && hasCurrentEndPos) {
        btVector3 currentStartPos = std::get<btVector3>(currentStartPosValue);
        btVector3 targetStartPos = std::get<btVector3>(currentStartPosValue);
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

    // Collision Normal (does not want interpolation.
    if (hasTargetCollisionNormal) {
        collisionNormal = std::get<btVector3>(targetCollisionNormalValue);
    }
}

std::vector<std::shared_ptr<Packet::Packet>> LaserObject::CreatePackets(int sequenceNum) {
    std::vector<std::shared_ptr<Packet::Packet>> packets;

    auto [read, readLock] = GetWorldStates()->GetReadState();

    StateValue startPosValue;
    StateValue endPosValue;
    StateValue collisionNormalValue;

    std::shared_lock readStateLock = read->Lock_Shared();
    
    bool hasStartPos = read->ReadState(StateType::StartPos, &startPosValue);
    bool hasEndPos = read->ReadState(StateType::EndPos, &endPosValue);
    bool hasCollisionNormal = read->ReadState(StateType::Normal, &collisionNormalValue);

    readStateLock.unlock();
    readLock.unlock();
    
    if (hasStartPos && hasEndPos && hasCollisionNormal) {
        packets.push_back(std::move(std::make_shared<Packet::LaserPacket>(
            GetWorldID(),
            std::get<btVector3>(startPosValue),
            std::get<btVector3>(endPosValue),
            std::get<btVector3>(collisionNormalValue),
            sequenceNum
        )));
    }

    return std::move(packets);
}
