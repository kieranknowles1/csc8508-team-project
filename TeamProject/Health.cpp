#include "Health.h"
#include "Multiplayer/GamePackets.hpp"
#include "WorldState.h"
#include "PlayerObject.h"
#include "Multiplayer/GamePackets.hpp"

using namespace WorldState;

namespace NCL {
    namespace CSC8503 {
        void HealthAttrib::Update(float dt) {
            elapsed += dt;

            // Regenerating HealthAttrib.
            if (GetHealthState() == AliveState::ALIVE && elapsed - lastHit >= regenDelay) {
                SetCurrentHealth(currentHealth + (regenRate * dt));
            }
        }

        void HealthAttrib::UpdateWorldState() {
            auto [writeState, writeLock] = GetWorldStates()->GetWriteState();
            std::unique_lock writeStateLock(writeState->Lock());
            writeState->UpdateState(StateType::Health, currentHealth);

            if (deaths > lastRecordedDeaths) {
                if (lastKilledBy) {
                    writeState->UpdateState(StateType::ObjectID, lastKilledBy->GetWorldID());
                }
                lastRecordedDeaths = deaths;
            }
        }

        void HealthAttrib::UpdateFromWorldState(float tickProgress) {
            std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };
            float weight = tickProgress;

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

        std::vector<std::shared_ptr<Packet::Packet>> HealthAttrib::CreatePackets(int sequenceNum) {
            std::vector<std::shared_ptr<Packet::Packet>> packets;
            auto [read, readLock] = GetWorldStates()->GetReadState();

            StateValue killedByValue;

            std::shared_lock readStateLock = read->Lock_Shared();
            bool hasKilledBy = read->ReadState(StateType::ObjectID, &killedByValue);

            readStateLock.unlock();
            readLock.unlock();

            if (hasKilledBy) {
                packets.push_back(std::move(std::make_shared<Packet::DeathPacket>(
                    std::get<int>(killedByValue),
                    100.0f,
                    sequenceNum
                )));
            }
            return packets;
        }

        void HealthAttrib::Damage(float amount) {
            if (elapsed - lastSpawn >= invulnerableWindow) {
                std::scoped_lock lock(damageMutex);
                float newHealthAttrib = currentHealth - amount;
                SetCurrentHealth(newHealthAttrib);
                lastHit = elapsed;
            }
        }

        /**
         * @brief Updates attributes in this object based on the damage and
         * damage type.
         *
         * DISCRETE damage is applied once and then target is reset to nullptr.
         * CONTINUOUS damage is multiplied by dt and the target is not reset.
         *
         * Damage is tracked by pairs noting who was hit and for how much
         * damage. The network creates a damage packet for each pair and resets
         * the array.
         */
        void AttackAttrib::Update(float dt) {
            if (target) {
                float damageTotal;

                switch (damageType) {
                case DamageType::DISCRETE:
                    damageTotal = damage;
                    break;

                case DamageType::CONTINUOUS:
                    damageTotal = damage * dt;
                    break;

                default:
                    damageTotal = 0;
                    break;
                }
                target->Damage(damageTotal);

                std::unique_lock lock(hitsMutex);
                hits.push_back(std::pair(target->GetParent(), damageTotal));
                lock.unlock();

                // If hit is single hit, reset target to prevent more hits.
                if (damageType == DamageType::DISCRETE) target = nullptr;
            }
        }

        void AttackAttrib::UpdateWorldState() {
            // Nothing to do. State is already updates by Update() function
            // When hitting things.
        }


        // Receives incoming damage states from other players.
        void AttackAttrib::UpdateFromWorldState(float dt) {
            // Nothing to do. State is already updates from packets and object
            // is locked when writing to hits array.
        }

        /**
         * @brief Creates a damage packet for each hit registered in the
         * internal buffer.
         * 
         * Temporarily locks the hitsMutex using a shared_lock.
         * Clears the hits vector.
         */
        std::vector<std::shared_ptr<Packet::Packet>> AttackAttrib::CreatePackets(int sequenceNum) {
            std::vector<std::shared_ptr<Packet::Packet>> packets;
            std::shared_lock lock(hitsMutex);
    
            for (auto [target, damage] : hits) {
                std::shared_ptr<Packet::DamagePacket> damagePacket = std::make_shared<Packet::DamagePacket>(
                    target->GetWorldID(),
                    damage,
                    health->GetParent()->GetWorldID()
                );
                packets.push_back(damagePacket);
            }
            hits.clear();
            return std::move(packets);
        }
    }
}

