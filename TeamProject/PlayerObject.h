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

// Player class derived from GameObject
class PlayerObject : public GameObject {
public:
	void Update(float dt) override;

	void OnCollisionEnter(const CollisionInfo& collisionInfo) override;

	void OnCollisionExit(const CollisionInfo& collisionInfo) override;

	void OnCollisionStay(const CollisionInfo& collision) override;

	void updateGravity(float dt);

	btVector3 getUpDirection() {
		return upDirection;
	}

	btVector3 getRightDirection() {
		return rightDirection;
	}

	btVector3 getForwardDirection() {
		return forwardDirection;
	}

	btQuaternion getCamOffset() {
		return camRotOffset;
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

	btVector3 getCollisionNormal() {
		return collisionNormal;
	}
	btVector3 getCollisionPoint() {
		return collisionPoint;
	}
	void Rotate(bool positive, bool rolling, float yaw);

	/**
	 * @brief Get the state of the player (usually alive or dead).
	 * @return PlayerState Enum
	 */
	inline PlayerState GetState() { return state; }

private:



	//Player Variables
	float gravityScale = 400.0f;
	float rotateTime = 0.5f;

	int collided = 0;
	btVector3 collisionNormal = btVector3(0, 1, 0);
	btVector3 collisionPoint = btVector3(0, 0, 0);
	std::list<GameObject*> collidedObjects;
	char collisionType;
	PlayerState state;

	btQuaternion camRotOffset = btQuaternion::getIdentity();
	btQuaternion oldcamRotOffset = btQuaternion::getIdentity();
	btQuaternion targetcamRotOffset = btQuaternion::getIdentity();
	btVector3 targetWorldRotation = btVector3(0, 1, 0);
	btVector3 oldWorldRotation = btVector3(0, 1, 0);
	btVector3 upDirection;
	btVector3 rightDirection;
	btVector3 forwardDirection;
	float rotateTimer = 0.0f;
	bool rotationChanging = false;

	void CalculateDirections(float dt);
	btVector3 CalculateRightDirection(btVector3 upDir);
	btVector3 CalculateForwardDirection(btVector3 upDir, btVector3 rightDir);
	btVector3 CalculateUpDirection(float dt);
	btVector3 CalculateForwardFromYaw(float yaw);
	btVector3 CalculateRightFromYaw(float yaw);

};
