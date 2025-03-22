#pragma once

#include "GameObject.h"

namespace NCL::CSC8503 {
    enum class HealthState {
        ALIVE,
        DEAD
    };

    class HitPointedEntity : public GameObject {
    public:
        void Update(float dt) override;

        /**
         * @brief This function changes the current health by the given amount.
         * The current health value is clamped between 0 and maxHealth.
         */
        void Damage(float amount);

        void SetMaxHealth(float health) { maxHealth = health; }
        float GetMaxHealth() const { return maxHealth; }

        void SetCurrentHealth(float healt) { currentHealth = std::clamp(health, 0.0f, maxHealth); }
        float GetCurrentHealth() const { return currentHealth; }

    private:
        float maxHealth = 0;
        float currentHealth = 0;

        HealthState state = HealthState::DEAD;
    };


    enum class DamageType {
        CONTINUOUS,     // Does not reset on hit.
        DISCRETE        // Resets on damage dealt.
    };

    class AttackerEntity : public GameObject {
    public:
        void Update(float dt) override;

        void SetDamageType(DamageType type) { damageType = type; }
        DamageType GetDamageType() const { return damageType; }

        void SetDamageAmount(float amount) { damage = amount; }
        float GetDamageAmount() const { return damage; }

        /**
         * @brief Defines the target to hit next time Update is called.
         * Use nullptr to hit no one. Is reset to nullptr after Update if
         * damage type is DISCRETE.
         */
        void Hit(HitPointedEntity* target) { target = target; }

    private:
        float damage = 0;
        DamageType damageType = DamageType::DISCRETE;
        HitPointedEntity* target = nullptr;
    };
}
