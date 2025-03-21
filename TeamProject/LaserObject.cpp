#include "LaserObject.h"
#include "Multiplayer/WorldState.hpp"
#include "Multiplayer/GamePackets.hpp"
#include "GameTechRenderer.h"
#include "Shoot.h"


using namespace WorldState;
using namespace NCL::CSC8503;


void LaserObject::UpdateObjectState() {
    auto [writeState, lock] = states->GetWriteState();
    
    std::unique_lock stateLock = writeState->Lock();
    writeState->UpdateState(StateType::StartPos, startPos);
    writeState->UpdateState(StateType::EndPos, endPos);
    writeState->UpdateState(StateType::Normal, collisionNormal);
}

void LaserObject::UpdateFromState(float dt) {
    elapsedTickTime += dt;

    std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };
    float weight = TICK_UPDATE_RATE / fmod(elapsedTickTime, TICK_UPDATE_RATE);

    auto [current, currentLock] = states->GetCurrentState();
    auto [read, readLock] = states->GetReadState();

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

    bool hasCurrentCollisionNormal = current->ReadState(StateType::Normal, &currentCollisionNormalValue);
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


    collisionNormal = std::get<btVector3>(targetCollisionNormalValue);

    // End Position.
    if (hasCurrentEndPos && hasCurrentEndPos && collisionNormal && initialised) {
        btVector3 currentEndPos = std::get<btVector3>(currentEndPosValue);
        btVector3 targetEndPos = std::get<btVector3>(currentEndPosValue);
        btVector3 interpolated = btVector3(
            lerp(currentEndPos.x(), targetEndPos.x(), weight),
            lerp(currentEndPos.y(), targetEndPos.y(), weight),
            lerp(currentEndPos.z(), targetEndPos.z(), weight)
        );
        endPos = interpolated;

        Shoot::GetInstance()->SpawnDecal(endPos, collisionNormal, parent->GetWorldID());
    } 


    
}

std::vector<std::shared_ptr<Packet::Packet>> LaserObject::CreatePackets(int sequenceNum) {
    std::vector<std::shared_ptr<Packet::Packet>> packets;

    auto [read, readLock] = states->GetReadState();

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

    return packets;
}
