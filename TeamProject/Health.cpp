#include "Health.h"

using namespace NCL::CSC8503;


void HitPointedEntity::Update(float dt) {
    state = currentHealth <= 0 ? HealthState::DEAD : HealthState::ALIVE;
}


void HitPointedEntity::Damage(float amount) { 
    float newHealth = currentHealth - health; 
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
