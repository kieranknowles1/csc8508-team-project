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
		if (otherObject->getIsFloor()) {
			collided++;
		}
	}
	void OnCollisionExit(GameObject* otherObject) override {
		if (otherObject->getIsFloor()) {
			collided--;
		}
	}
	void setCollided(int collidedIn) {
		collided = collidedIn;
	}
	int getCollided() {
		return collided;
	}
private:
	int collided = 0;
};
