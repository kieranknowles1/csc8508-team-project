#include "Health.h"
#include "Multiplayer/GamePackets.hpp"
#include "WorldState.h"
#include "PlayerObject.h"

using namespace WorldState;

namespace NCL {
    namespace CSC8503 {
        void HealthAttrib::Update(float dt) {
            elapsed += dt;

            // Regenerating HealthAttrib.
            if (GetHealthState() == HealthState::ALIVE) {
                SetCurrentHealth(currentHealth+ (regenRate * dt));
            }
        }

        void HealthAttrib::UpdateWorldState() {
            auto [writeState, writeLock] = GetWorldStates()->GetWriteState();
            std::unique_lock writeStateLock(writeState->Lock());
            writeState->UpdateState(StateType::Health, currentHealth);
        }

        void HealthAttrib::UpdateFromWorldState(float dt) {
            elapsedTickTime += dt;

            std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };
            float weight = fmod(elapsedTickTime, TICK_UPDATE_RATE) / TICK_UPDATE_RATE;

            auto [current, currentLock] = GetWorldStates()->GetCurrentState();
            auto [read, readLock] = GetWorldStates()->GetReadState();

            StateValue currentHealthAttrib;
            StateValue targetHealthAttrib;

            std::shared_lock currentStateLock = current->Lock_Shared();
            std::shared_lock readStateLock = read->Lock_Shared();

            bool hasCurrentHealthAttrib = current->ReadState(StateType::Health, &currentHealthAttrib);
            bool hasTargetHealthAttrib = read->ReadState(StateType::Health, &targetHealthAttrib);

            currentStateLock.unlock();
            readStateLock.unlock();

            currentLock.unlock();
            readLock.unlock();

            // Updating health.
            if (hasCurrentHealthAttrib && hasTargetHealthAttrib) {
                float current = std::get<float>(currentHealthAttrib);
                float target = std::get<float>(targetHealthAttrib);
                SetCurrentHealth(lerp(current, target, weight));
            }
        }

        std::vector<std::shared_ptr<Packet::Packet>> HealthAttrib::CreatePackets(int seuqnceNum) {
            return {};
        }

        void HealthAttrib::Damage(float amount) {
            float newHealthAttrib = currentHealth- amount;
            SetCurrentHealth(newHealthAttrib);
            lastHit = elapsed;
        }

        void AttackAttrib::Update(float dt) {
            if (target) {
                switch (damageType) {
                case DamageType::DISCRETE:

                    target->Damage(damage);
                    damageDealt += damage;
                    break;

                case DamageType::CONTINUOUS:
                    target->Damage(damage * dt);
                    damageDealt += (damage * dt);
                    break;

                default:
                    damageDealt = 0;
                    break;
                }
            }
        }

        void AttackAttrib::UpdateWorldState() {
            auto [writeState, lock] = GetWorldStates()->GetWriteState();

            std::unique_lock stateLock(writeState->Lock());

            writeState->UpdateState(StateType::DamageDealt, damageDealt);
            writeState->UpdateState(StateType::ObjectID, lastHit->GetWorldID());
        }

        // Receives incoming damage states from other players.
        void AttackAttrib::UpdateFromWorldState(float dt) {
            elapsedTickTime += dt;
            float percent = dt / TICK_UPDATE_RATE;

            auto [read, readLock] = health->GetWorldStates()->GetReadState();
            StateValue targetDamageDealt;

            std::shared_lock readStateLock = read->Lock_Shared();
            bool hasTargetDamageDealt = read->ReadState(StateType::DamageDealt, &targetDamageDealt);
            readStateLock.unlock();

            // Damaging over time.
            if (hasTargetDamageDealt) {
                health->Damage(std::get<float>(targetDamageDealt) * percent);
            }
        }

        std::vector<std::shared_ptr<Packet::Packet>> AttackAttrib::CreatePackets(int sequenceNum) {
            
            
            return {};
        }
    }
}
