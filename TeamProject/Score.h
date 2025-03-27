#pragma once

#include "StateUpdater.h"

#include <string>

namespace NCL::CSC8503 {
    class GameObject;

    class ScoreAttrib : public StateUpdater {
    public:
        ScoreAttrib(GameObject* parent) : parent(parent) {}

        virtual void UpdateWorldState();
        virtual void UpdateFromWorldState(float dt);
        virtual std::vector<std::shared_ptr<Packet::Packet>> CreatePackets(int sequenceNum);
        
        void AddToScore(float amount) { score += (amount * multiplier); }
        void SetScore(float s) { score = s; }
        float GetScore() { return score; }

        void SetMultiplier(float mult) { multiplier = mult; }
        float GetMultiplier() { return multiplier; }

        void Reset() { score = 0; multiplier = 1.0f; }
        std::string String() { return std::to_string((int) score); }

        GameObject* GetParent() { return parent; }

        bool operator<(const ScoreAttrib& other) const { return score < other.score; }

    private:
        GameObject* parent  = nullptr;

        float score         = 0.0f;
        float multiplier    = 1.0f;
    };
}
