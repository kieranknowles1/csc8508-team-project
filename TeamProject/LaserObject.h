#pragma once

#include "GameObject.h"

namespace NCL::CSC8503 {

    class LaserObject : public GameObject {
    public:
        LaserObject(float thickness, btVector4 color) : thickness(thickness), color(color) {}

        void UpdateObjectState() override;
        void UpdateFromState(float dt) override;

        void SetCollisionNormal(btVector3 n) { collisionNormal = n; }
        void SetStartPos(btVector3 s) { startPos = s; }
        void SetEndPos(btVector3 e) { endPos = e; }
        void SetColor(btVector4 c) { color = c; }
        void SetThickness(float t) { thickness = t; }

        btVector3& GetCollisionNormal() { return collisionNormal; }
        btVector3& GetStartPos() { return startPos; }
        btVector3& GetEndPos() { return endPos; }
        btVector4 GetColor() const { return color; }
        float GetThickness() const { return thickness; }

    private:
        btVector3 collisionNormal;
        btVector3 startPos;
        btVector3 endPos;
        btVector4 color;
        float thickness;
    };
}
