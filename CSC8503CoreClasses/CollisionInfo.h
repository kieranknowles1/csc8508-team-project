#pragma once

#include "btBulletDynamicsCommon.h"

namespace NCL::CSC8503 {
	class GameObject;

	struct CollisionInfo {
		GameObject* otherObject; // The other object involved in the collision
		btVector3 contactPointA; // The contact point on the parent object
		btVector3 contactPointB; // The contact point on the other object
		btVector3 contactNormal; // The contact normal
		float penetrationDepth; // The penetration depth
		btVector3 relativeVelocity; // The relative velocity of the two objects

		bool operator==(const CollisionInfo& other) const {
			return otherObject == other.otherObject;
		}
	};

	bool operator<(const CollisionInfo& lhs, const CollisionInfo& rhs);

	bool operator<(const GameObject* lhs, const CollisionInfo& rhs);

	bool operator<(const CollisionInfo& lhs, const GameObject* rhs);
}
