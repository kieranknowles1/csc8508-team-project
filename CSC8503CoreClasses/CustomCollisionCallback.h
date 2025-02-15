#pragma once

#include "GameObject.h"
#include "btBulletDynamicsCommon.h"
#include "CustomCollisionCallback.h"
#include <unordered_set>

namespace NCL {
	namespace CSC8503 {
		class CustomCollisionCallback : public btCollisionWorld::ContactResultCallback {
		public:
			// Using an unordered set to store the active collisions
			// This set just stores detailed per-frame collision information and is temporary
			std::unordered_set<CollisionInfo> activeCollisions;

			CustomCollisionCallback(GameObject* parentObject) : parent(parentObject) {}

			// This is called for each contact point
			btScalar addSingleResult(btManifoldPoint& cp,
				const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
				const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override {
				
				GameObject* obj0 = (GameObject*)colObj0Wrap->getCollisionObject()->getUserPointer();
				GameObject* obj1 = (GameObject*)colObj1Wrap->getCollisionObject()->getUserPointer();

				// Check if the parent object is involved in the collision
				if (obj0 == parent || obj1 == parent) {
					// create a CollisionInfo struct to store the collision information
					CollisionInfo collisionInfo;
					collisionInfo.otherObject = (obj0 == parent) ? obj1 : obj0;
					collisionInfo.contactPointA = cp.getPositionWorldOnA();
					collisionInfo.contactPointB = cp.getPositionWorldOnB();
					collisionInfo.contactNormal = cp.m_normalWorldOnB;
					collisionInfo.penetrationDepth = cp.getDistance();
					collisionInfo.relativeVelocity = obj0->GetPhysicsObject()->GetRigidBody()->getLinearVelocity() - obj1->GetPhysicsObject()->GetRigidBody()->getLinearVelocity();

					// Call the OnCollisionEnter method of the parent object
					parent->OnCollisionEnter(collisionInfo);

					// Print the contact points for debugging
					//std::cout << "Contact Point A: " << contactPointA.getX() << " " << contactPointA.getY() << " " << contactPointA.getZ() << std::endl;
					//std::cout << "Contact Point B: " << contactPointB.getX() << " " << contactPointB.getY() << " " << contactPointB.getZ() << std::endl;
				
					// Add the object's collision info to the active collisions set
					
					activeCollisions.insert(collisionInfo);
				}

				return 0;
			}

		protected:
			GameObject* parent;
		};
	}
}
