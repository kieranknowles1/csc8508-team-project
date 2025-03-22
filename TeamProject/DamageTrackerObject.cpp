#include "DamageTrackerObject.h"
#include "WorldState.h"
#include "Multiplayer/GamePackets.hpp"


using namespace WorldState;
using namespace NCL::CSC8503;


void DamageTrackerObject::Update(float dt) {
    // Handle it's state here with the respawning and scores.
}


void DamageTrackerObject::UpdateWorldState() {
    auto [readState, readLock] = GetWorldStates()->GetReadState();
    auto [writeState, writeLock] = GetWorldStates()->GetWriteState();
    
    StateValue previousHealth;
    StateValue previousHealthState;

    std::shared_lock readStateLock = readState->Lock_Shared();
    std::unique_lock writeStateLock = writeState->Lock();

    // Read state will contain health at last server tick.
    if (readState->ReadState(StateType::Health, &previousHealth)) {
        float damageTaken = std::get<float>(previousHealth) - health;
        writeState->UpdateState(StateType::DamageTaken, damageTaken);
    }
}

void DamageTrackerObject::UpdateFromWorldState(float dt) {
    elapsedTickTime += dt;

    std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };
    float weight = TICK_UPDATE_RATE / fmod(elapsedTickTime, TICK_UPDATE_RATE);

    auto [current, currentLock] = GetWorldStates()->GetCurrentState();
    auto [read, readLock] = GetWorldStates()->GetReadState();

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

    auto [read, readLock] = GetWorldStates()->GetReadState();

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
