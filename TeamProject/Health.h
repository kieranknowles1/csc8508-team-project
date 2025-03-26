#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>

#include "StateUpdater.h"
#include "AliveState.h"

namespace NCL::CSC8503 {
    class GameObject;

    /**
     * @brief An entity that can be damaged.
     *
     * This entity is to be used as an abstract class to inherit from. It does
     * NOT update GameObject states. If you want state updates for GameObject,
     * you must explicitly call base class update functions.
     */
    class HealthAttrib : public StateUpdater {
    public:
        HealthAttrib(GameObject* parent) : parent(parent) {}

        void Update(float dt);

        void UpdateWorldState() override;
        void UpdateFromWorldState(float dt) override;
        std::vector<std::shared_ptr<Packet::Packet>> CreatePackets(int sequenceNum) override;

        /**
         * @brief This function changes the current health by the given amount.
         * The current health value is clamped between 0 and maxHealth.
         */
        void Damage(float amount);

        void SetMaxHealth(float health) { maxHealth = health; }
        float GetMaxHealth() const { return maxHealth; }

        void SetCurrentHealth(float health) { currentHealth = std::clamp(health, 0.0f, maxHealth); }
        float GetCurrentHealth() const { return currentHealth; }

        /**
         * @brief Set how much HP to regenerate per second.
         */
        void SetRegenerationRate(float rate) { regenRate = rate; }
        float GetRegenerationRate() const { return regenRate; }

        /**
         * @brief Set the wait time before healing after taking damage.
         */
        void SetRegenerationDelay(float delay) { regenDelay = delay; }
        float GetRegenerationDelay() const { return regenDelay; }

        void SetInvulnerableWindow(float time) { invulnerableWindow = time; }
        float GetInvulernableWindow() { return invulnerableWindow; }

        void Respawn() {
            SetCurrentHealth(GetMaxHealth());
            lastSpawn = elapsed;
        }

        AliveState GetHealthState() { return currentHealth == 0 ? AliveState::DEAD : AliveState::ALIVE; }
        GameObject* GetParent() { return parent; }

    protected:
        GameObject* parent;
        std::mutex damageMutex;

        float maxHealth = 0;
        float currentHealth = 0;
        float regenRate = 0;
        float regenDelay = 0;
        float invulnerableWindow = 0;

        float elapsed = 0;
        float lastHit = 0;
        float lastSpawn = 0;
    };


    enum class DamageType {
        CONTINUOUS,     // Does not reset on hit.
        DISCRETE        // Resets on damage dealt.
    };

    class AttackAttrib : public StateUpdater {
    public:
        void Update(float dt);

        void UpdateWorldState() override;
        void UpdateFromWorldState(float dt) override;
        std::vector<std::shared_ptr<Packet::Packet>> CreatePackets(int sequenceNum) override;

        void SetDamageType(DamageType type) { damageType = type; }
        DamageType GetDamageType() const { return damageType; }

        void SetDamageAmount(float amount) { damage = amount; }
        float GetDamageAmount() const { return damage; }

        /**
         * @brief Defines the target to hit next time Update is called.
         * Use nullptr to hit no one. Is reset to nullptr after Update if
         * damage type is DISCRETE.
         */
        void Hit(HealthAttrib* t) { target = t; }

        void SetHealthAttrib(HealthAttrib* health) { this->health = health; }
        HealthAttrib* GetHealthAttrib() { return health; }

        std::pair<std::vector<std::pair<GameObject*, float>>&, std::unique_lock<std::shared_mutex>> GetWriteableHits() {
            std::unique_lock lock(hitsMutex);
            return std::pair<std::vector<std::pair<GameObject*, float>>&, std::unique_lock<std::shared_mutex>>(hits, std::move(lock));
        }

    protected:
        float damage = 0;

        DamageType damageType = DamageType::DISCRETE;

        HealthAttrib* target = nullptr; // Reference to health of enemy.
        HealthAttrib* health = nullptr; // Reference to own health.

        std::vector<std::pair<GameObject*, float>> hits;
        std::shared_mutex hitsMutex; // Object State class doesn't support lists.
    };
}
