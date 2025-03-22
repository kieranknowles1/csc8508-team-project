#include "DamageTrackerObject.h"
#include "Multiplayer/WorldState.hpp"
#include "Multiplayer/GamePackets.hpp"


using namespace WorldState;
using namespace NCL::CSC8503;


void DamageTrackerObject::Update(float dt) {
}


void DamageTrackerObject::UpdateObjectState() {
    auto [writeState, lock] = states->GetWriteState();
    
    std::unique_lock stateLock = writeState->Lock();
    //writeState->UpdateState(StateType::Normal, collisionNormal);
}

void DamageTrackerObject::UpdateFromState(float dt) {
    elapsedTickTime += dt;

    std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };
    float weight = TICK_UPDATE_RATE / fmod(elapsedTickTime, TICK_UPDATE_RATE);

    auto [current, currentLock] = states->GetCurrentState();
    auto [read, readLock] = states->GetReadState();

    //StateValue currentStartPosValue;
    //StateValue targetStartPosValue;

    // Reading.
    std::shared_lock currentStateLock = current->Lock_Shared();
    std::shared_lock readStateLock = read->Lock_Shared();

    //bool hasCurrentStartPos = current->ReadState(StateType::StartPos, &currentStartPosValue);
    //bool hasTargetStartPos = read->ReadState(StateType::StartPos, &targetStartPosValue);

    currentStateLock.unlock();
    readStateLock.unlock();

    currentLock.unlock();
    readLock.unlock();

    //// Start Position.
    //if (hasCurrentStartPos && hasCurrentEndPos) {
    //    btVector3 currentStartPos = std::get<btVector3>(currentStartPosValue);
    //    btVector3 targetStartPos = std::get<btVector3>(currentStartPosValue);
    //    btVector3 interpolated = btVector3(
    //        lerp(currentStartPos.x(), targetStartPos.x(), weight),
    //        lerp(currentStartPos.y(), targetStartPos.y(), weight),
    //        lerp(currentStartPos.z(), targetStartPos.z(), weight)
    //    );
    //    startPos = interpolated;
    //} 
}

std::vector<std::shared_ptr<Packet::Packet>> DamageTrackerObject::CreatePackets(int sequenceNum) {
    std::vector<std::shared_ptr<Packet::Packet>> packets;

    auto [read, readLock] = states->GetReadState();

    //StateValue startPosValue;

    std::shared_lock readStateLock = read->Lock_Shared();
    
    //bool hasStartPos = read->ReadState(StateType::StartPos, &startPosValue);

    readStateLock.unlock();
    readLock.unlock();
    
    //if (hasStartPos && hasEndPos && hasCollisionNormal) {
    //    packets.push_back(std::move(std::make_shared<Packet::LaserPacket>(
    //        GetWorldID(),
    //        std::get<btVector3>(startPosValue),
    //        std::get<btVector3>(endPosValue),
    //        std::get<btVector3>(collisionNormalValue),
    //        sequenceNum
    //    )));
    //}

    return packets;
}
