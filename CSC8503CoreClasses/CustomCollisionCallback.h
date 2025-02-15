#pragma once

#include "GameObject.h"
#include "btBulletDynamicsCommon.h"

namespace NCL {
	namespace CSC8503 {
		class CustomCollisionCallback : public btCollisionWorld::ContactResultCallback {
		public:
			btVector3 contactPointA;
			btVector3 contactPointB;
			std::set<GameObject*> activeCollisions;

			CustomCollisionCallback(GameObject* parentObject) : parent(parentObject) {}

			// This is called for each contact point
			btScalar addSingleResult(btManifoldPoint& cp,
				const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
				const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override {
				GameObject* obj0 = (GameObject*)colObj0Wrap->getCollisionObject()->getUserPointer();
				GameObject* obj1 = (GameObject*)colObj1Wrap->getCollisionObject()->getUserPointer();

				/*
					Well I think it is not necessary to check which of the objects is the parent. Because the parent 
					object is the one that has the CustomCollisionCallback and the other object is the one that is collided 
					with the parent object so the parent object is always the first object in the collision. But I did it
					anyway, just to be not have unexpected results, like we had when we disabled rigidbody for the cube.

					And, this method is more robust as it works for two-way collision handling, in case we need it.
				*/
				// Check if the parent object is involved in the collision
				if (obj0 == parent || obj1 == parent) {
					// Get the contact points
					contactPointA = cp.getPositionWorldOnA();
					contactPointB = cp.getPositionWorldOnB();

					// Print the contact points for debugging
					//std::cout << "Contact Point A: " << contactPointA.getX() << " " << contactPointA.getY() << " " << contactPointA.getZ() << std::endl;
					//std::cout << "Contact Point B: " << contactPointB.getX() << " " << contactPointB.getY() << " " << contactPointB.getZ() << std::endl;
				
					// Add the object to the active collisions set
					GameObject* otherObject = (obj0 == parent) ? obj1 : obj0;
					activeCollisions.insert(otherObject);
				}

				return 0;
			}

		protected:
			GameObject* parent;
		};
	}
}
