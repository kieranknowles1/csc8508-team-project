#pragma once

#include "GameObject.h"

namespace NCL::CSC8503 {

    class LaserObject : public GameObject {
    public:
        LaserObject(GameObject* parent) : parent(parent) {}

        void Update(float dt) override;

        void UpdateWorldState() override;
        void UpdateFromWorldState(float dt) override;
        std::vector<std::shared_ptr<Packet::Packet>> CreatePackets(int sequenceNum) override;

        void SetCollisionNormal(btVector3 n) { collisionNormal = n; }
        void SetStartPos(btVector3 s) { startPos = s; }
        void SetEndPos(btVector3 e) { endPos = e; }
        void SetColor(btVector4 c) { color = c; }
        void SetThickness(float t) { thickness = t; }

        btVector3& GetCollisionNormal() { return collisionNormal; }
        btVector3& GetStartPos() { return startPos; }
        btVector3& GetEndPos() { return endPos; }
        btVector4& GetColor() { return color; }
        float GetThickness() const { return thickness; }

    private:
        btVector3 collisionNormal;
        btVector3 startPos;
        btVector3 endPos;
        btVector4 color;
        float thickness;
        GameObject* parent;
    };

    /**
     * @brief Compare 2 pointers to LaserObjects.
     * For user by renderer for tracking lasers.
     */
    class LaserComparator {
    public:
        bool operator()(const LaserObject* laser1, const LaserObject* laser2) const {
            return laser1->GetWorldID() < laser2->GetWorldID();
        }
    };
}
