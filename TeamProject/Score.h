#pragma once

#include <string>

namespace NCL::CSC8503 {
    class GameObject;

    class ScoreAttrib {
    public:
        ScoreAttrib(GameObject* parent) : parent(parent) {}

        void AddToScore(float amount) { score += (amount * multiplier); }
        float GetScore() { return score; }

        void SetMultiplier(float mult) { multiplier = mult; }
        float GetMultiplier() { return multiplier; }

        void Reset() { score = 0; multiplier = 1.0f; }
        std::string String() { return std::to_string((int) score); }

        bool operator<(const ScoreAttrib& other) const { return score < other.score; }

    private:
        GameObject* parent  = nullptr;

        float score         = 0.0f;
        float multiplier    = 1.0f;
    };
}
