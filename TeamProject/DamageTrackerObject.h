#pragma once

#include "GameObject.h"

namespace NCL::CSC8503 {

    class DamageTrackerObject : public GameObject {
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
