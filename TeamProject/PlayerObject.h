#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"
#include "PhysicsObject.h"
#include "CollisionInfo.h"


#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

using namespace NCL::CSC8503;
// Paintball class derived from GameObject
class PlayerObject : public GameObject {
public:
	void OnCollisionEnter(const CollisionInfo& collisionInfo) override {
		btVector3 playerPos = this->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
		btVector3 objPos = collisionInfo.contactPointA;
		btVector3 direction = (objPos - playerPos).normalized();
		float dot = direction.dot(-upDirection);
		float angle = acos(dot) * (180.0f / SIMD_PI);
		if (angle <= 30.0f) {
			collided++;
			collidedObjects.push_back(collisionInfo.otherObject);
		}
	}


	void OnCollisionExit(const CollisionInfo& collisionInfo) override {
		auto it = std::find(collidedObjects.begin(), collidedObjects.end(), collisionInfo.otherObject);
		if (it != collidedObjects.end()) {
			collidedObjects.erase(it);
			if (collided > 0) {
				collided--;
			}
		}
	}

	void OnCollisionStay(const CollisionInfo& collision) override {
		btVector3 playerPos = this->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
		btVector3 objPos = collision.contactPointA;
		btVector3 direction = (objPos - playerPos).normalized();
		float dot = direction.dot(-upDirection);
		float angle = acos(dot) * (180.0f / SIMD_PI);
		auto it = std::find(collidedObjects.begin(), collidedObjects.end(), collision.otherObject);
		if (it != collidedObjects.end()) { // contains already
			if (angle >= 30.0f) { // now too steep for floor
				collidedObjects.erase(it);
				if (collided > 0) {
					collided--;
				}
			}
		}
		else { // not counted as floor yet
			if (angle <= 30.0f) {
				collided++;
				collidedObjects.push_back(collision.otherObject);
			}
		}
	}


	void setCollided(int collidedIn) {
		collided = collidedIn;
	}
	int getCollided() {
		return collided;
	}
	void setUpDirection(btVector3 upDirectionIn) {
		upDirection = upDirectionIn;
	};
private:
	int collided = 0;
	btVector3 upDirection;
	std::list<GameObject*> collidedObjects;
};
