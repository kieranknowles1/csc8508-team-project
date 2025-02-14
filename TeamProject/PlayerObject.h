#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"
#include "PhysicsObject.h"


#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

using namespace NCL::CSC8503;
// Paintball class derived from GameObject
class PlayerObject : public GameObject {
public:
	void OnCollisionEnter(GameObject* otherObject) override {
		btVector3 playerPos = this->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
		btVector3 objPos = otherObject->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
		btVector3 direction = (objPos - playerPos).normalize();
		float dotProduct = direction.dot(upDirection);
		if (dotProduct < 0.0f) {
			collided++;
		}
	}
	void OnCollisionExit(GameObject* otherObject) override {
		if (collided > 0) {
			collided--;
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
