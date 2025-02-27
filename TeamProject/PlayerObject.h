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


enum class PlayerState {
	DEAD,
	ALIVE
};


// Paintball class derived from GameObject
class PlayerObject : public GameObject {
public:
	void OnCollisionEnter(const CollisionInfo& collisionInfo) override {
		if (collisionInfo.otherObject->getIsPaintball()) return;
		btVector3 playerPos = this->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
		btVector3 objPos = collisionInfo.contactPointA;
		btVector3 direction = (objPos - playerPos).normalized();
		float dot = direction.dot(-upDirection);
		float angle = acos(dot) * (180.0f / SIMD_PI);
		if (angle <= 25.0f) {
			collided++;
			collidedObjects.push_back(collisionInfo.otherObject);
		}

		// set special type collision
		collisionType = collisionInfo.otherObject->getType();
		if (collisionInfo.otherObject->getType() == 'J' || collisionInfo.otherObject->getType() == 'S') {
			collisionNormal = collisionInfo.contactNormal;
			collisionPoint = collisionInfo.contactPointA;
		}
	}


	void OnCollisionExit(const CollisionInfo& collisionInfo) override {
		if (collisionInfo.otherObject->getIsPaintball()) return;
		auto it = std::find(collidedObjects.begin(), collidedObjects.end(), collisionInfo.otherObject);
		if (it != collidedObjects.end()) {
			collidedObjects.erase(it);
			if (collided > 0) {
				collided--;
			}
		}
	}

	void OnCollisionStay(const CollisionInfo& collision) override {
		if (collision.otherObject->getIsPaintball()) return;
		btVector3 playerPos = this->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
		btVector3 objPos = collision.contactPointA;
		btVector3 direction = (objPos - playerPos).normalized();
		float dot = direction.dot(-upDirection);
		float angle = acos(dot) * (180.0f / SIMD_PI);
		auto it = std::find(collidedObjects.begin(), collidedObjects.end(), collision.otherObject);
		if (it != collidedObjects.end()) { // contains already
			if (angle > 25.0f) { // now too steep for floor
				collidedObjects.erase(it);
				if (collided > 0) {
					collided--;
				}
			}
		}
		else { // not counted as floor yet
			if (angle <= 25.0f) {
				collided++;
				collidedObjects.push_back(collision.otherObject);
			}
		}
		// set special type collision
		collisionType = collision.otherObject->getType();
		if (collision.otherObject->getType() == 'J' || collision.otherObject->getType() == 'S') {
			collisionNormal = collision.contactNormal;
			collisionPoint = collision.contactPointA;
		}
	}

	char getType() {
		return collisionType;
	}
	void resetType() {
		collisionType = 'N'; //type None
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
	btVector3 getCollisionNormal() {
		return collisionNormal;
	}
	btVector3 getCollisionPoint() {
		return collisionPoint;
	}

	/**
	 * @brief Get the state of the player (usually alive or dead).
	 * @return PlayerState Enum
	 */
	inline PlayerState GetState() { return state; }

private:
	int collided = 0;
	btVector3 upDirection;
	btVector3 collisionNormal = btVector3(0, 1, 0);
	btVector3 collisionPoint = btVector3(0, 0, 0);
	std::list<GameObject*> collidedObjects;
	char collisionType;
	PlayerState state;
};
