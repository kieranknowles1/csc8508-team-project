#pragma once

#include "GameObject.h"
#include "StateUpdater.h"

namespace NCL::CSC8503 {
    class ServerObject : public GameObject, public StateUpdater {
    public:
        ServerObject() {
            isNetworked = true;
        }

        virtual void UpdateWorldState() override;
        virtual void UpdateFromWorldState(float dt) override;
        virtual std::vector<std::shared_ptr<Packet::Packet>> CreatePackets(int sequenceNum) override;
    };
}
