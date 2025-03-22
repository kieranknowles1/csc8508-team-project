#include "Health.h"
#include "Multiplayer/GamePackets.hpp"
#include "WorldState.h"

using namespace NCL::CSC8503;
using namespace WorldState;

void Health::Update(float dt) {
    // Regenerating Health.
    if (GetHealthState() == HealthState::ALIVE) {
        SetCurrentHealth(currentHealth + (regenRate * dt));
    }
}

void Health::UpdateWorldState() {
    auto [writeState, writeLock] = GetWorldStates()->GetWriteState();
    std::unique_lock writeStateLock(writeState->Lock());
    writeState->UpdateState(StateType::Health, currentHealth);
}

void Health::UpdateFromWorldState(float dt) {
    elapsedTickTime += dt;

    std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };
    float weight = fmod(elapsedTickTime, TICK_UPDATE_RATE) / TICK_UPDATE_RATE;

    auto [read, readLock] = GetWorldStates()->GetReadState();

    StateValue targetHealth;
}

std::vector<std::shared_ptr<Packet::Packet>> CreatePackets(int seuqnceNum) {
    return {};
}

void Health::Damage(float amount) { 
    float newHealth = currentHealth - amount; 
    SetCurrentHealth(newHealth);
}


void AttackerEntity::Update(float dt) {
    if (target) {
        switch (damageType) {
        case DamageType::DISCRETE:
            target->Damage(damage);
            target = nullptr;
            break;

        case DamageType::CONTINUOUS:
            target->Damage(damage * dt);
            break;

        default:
            break;
        }
    }
}
