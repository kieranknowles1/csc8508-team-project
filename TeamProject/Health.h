#pragma once

#include "GameObject.h"

namespace NCL::CSC8503 {
    enum class HealthState {
        ALIVE,
        DEAD
    };

    class HitPointedEntity : public GameObject {
        HitPointedEntity() {}

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
        float maxHealth;
        float currentHealth;

        HealthState state;
    };
}
