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
	void setUpDirection(btVector3 upDirectionIn) {
		upDirection = upDirectionIn;
	};

	GameObject* getGround(btDiscreteDynamicsWorld* world);
	float getGroundRayOffset() { return groundRayOffset; };
private:
	// This + height
	float groundRayOffset = 0.25f;

	btVector3 upDirection;
	std::list<GameObject*> collidedObjects;
};
