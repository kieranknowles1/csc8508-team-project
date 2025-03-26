#include <memory>

#include "Score.h"
#include "Multiplayer/GamePackets.hpp"
#include "WorldState.h"
#include "GameObject.h"

using namespace WorldState;
using namespace NCL::CSC8503;


void ScoreAttrib::UpdateWorldState() {
    auto [writeState, lock] = GetWorldStates()->GetWriteState();

    std::unique_lock stateLock = writeState->Lock();
    writeState->UpdateState(StateType::Score, score);
}


void ScoreAttrib::UpdateFromWorldState(float tickProgress) {
    std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };

    auto [current, currentLock] = GetWorldStates()->GetCurrentState();
    auto [read, readLock] = GetWorldStates()->GetReadState();

    StateValue currentScoreValue;
    StateValue targetScoreValue;

    std::shared_lock currentStateLock = current->Lock_Shared();
    std::shared_lock readStateLock = read->Lock_Shared();

    bool hasCurrentScore = current->ReadState(StateType::Score, &currentScoreValue);
    bool hasTargetScore = read->ReadState(StateType::Score, &targetScoreValue);

    currentStateLock.unlock();
    readStateLock.unlock();

    // Write a score to the current lock so that it can interpolate.
    if (!hasCurrentScore && hasTargetScore) {
        std::unique_lock writeLock = current->Lock();
        current->UpdateState(StateType::Score, score);
        hasCurrentScore == true;
        currentScoreValue = score;
    }

    readLock.unlock();
    currentLock.unlock();

    // Lerp score between 2 values.
    if (hasCurrentScore && hasTargetScore) {
        float current = std::get<float>(currentScoreValue);
        float target = std::get<float>(targetScoreValue);

        score = lerp(current, target, tickProgress);
    }
}


std::vector<std::shared_ptr<Packet::Packet>> ScoreAttrib::CreatePackets(int sequenceNum) {
    std::vector<std::shared_ptr<Packet::Packet>> packets;

    std::shared_ptr<Packet::ScorePacket> scorePacket = std::make_shared<Packet::ScorePacket>(
        GetParent()->GetWorldID(),
        score,
        sequenceNum
    );
    packets.push_back(scorePacket);
    return std::move(packets);
}




