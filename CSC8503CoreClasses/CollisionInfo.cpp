#include "CollisionInfo.h"

namespace NCL::CSC8503 {
    bool operator<(const CollisionInfo& lhs, const CollisionInfo& rhs) {
        return lhs.otherObject < rhs.otherObject;
    }

    bool operator<(const GameObject* lhs, const CollisionInfo& rhs) {
        return lhs < rhs.otherObject;
    }

    bool operator<(const CollisionInfo& lhs, const GameObject* rhs) {
        return lhs.otherObject < rhs;
    }

}
