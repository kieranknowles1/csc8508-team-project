#pragma once

#include "Health.h"

namespace NCL::CSC8503 {
    /**
     * @brief Tracks how much damage is dealt to an entity.
     * 
     * Used in multiplayer to send damage packets and update the players
     * health. Each entity must have their own damage tracker.
     * 
     * AI currently use their own system and so this is not needed in
     * Singleplayer. Using this for the player is probably unnecessary.
     * 
     */
    class DamageTrackerObject : public HitPointedEntity {
    public:
        DamageTrackerObject(GameObject* parent) : parent(parent) {}

        void Update(float dt) override;

        void UpdateObjectState() override;
        void UpdateFromState(float dt) override;
        std::vector<std::shared_ptr<Packet::Packet>> CreatePackets(int sequenceNum) override;

    private:
        GameObject* parent;
    };
}
