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
			return otherObject == other.otherObject &&
				   contactPointA == other.contactPointA &&
				   contactPointB == other.contactPointB&&
				   contactNormal == other.contactNormal&&
				   penetrationDepth == other.penetrationDepth&&
				   relativeVelocity == other.relativeVelocity;
		} 
	};
}

// hash function to generate a hash value for the CollisionInfo struct
namespace std {
	template<>
	struct hash<btVector3> {
		size_t operator()(const btVector3& v) const {
			size_t h1 = hash<float>()(v.getX());
			size_t h2 = hash<float>()(v.getY());
			size_t h3 = hash<float>()(v.getZ());
			return h1 ^ (h2 << 1) ^ (h3 << 2); // combine the hash values
		}
	};

	template <>
	struct hash<NCL::CSC8503::CollisionInfo> {
		std::size_t operator()(const NCL::CSC8503::CollisionInfo& collision) const {
			std::size_t hash = 0;
			hash_combine(hash, collision.otherObject);
			hash_combine(hash, collision.contactPointA);
			hash_combine(hash, collision.contactPointB);
			hash_combine(hash, collision.contactNormal);
			hash_combine(hash, collision.penetrationDepth);
			hash_combine(hash, collision.relativeVelocity);
			return hash;
		}

		template <typename T>
		void hash_combine(std::size_t& seed, const T& v) const {
			std::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};
}