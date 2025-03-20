#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"
#include "PhysicsObject.h"
#include "CollisionInfo.h"
#include "Respawn.h"
#include "GameTechRendererInterface.h"

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <mutex>

namespace NCL::CSC8503 {

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

	void setRenderer(GameTechRendererInterface* rendIn) {
		renderer = rendIn;
	}

	void updateLaser(btVector3 startPos, btVector3 endPos) {
		renderer->updateLaser(GetWorldID(), startPos, endPos);
	}


	void setUpDirection(btVector3 target) {
		upDirection = target;
		targetWorldRotation = target;
		oldWorldRotation = target;
		btVector3 worldUp(0, 1, 0);

		if (upDirection.fuzzyZero() || upDirection == worldUp) {
			targetcamRotOffset = btQuaternion::getIdentity();
		}
		else if (upDirection == -worldUp) {
			// Special case: If target is exactly opposite of worldUp
			// Rotate 180 degrees around any perpendicular axis, e.g., X-axis (1,0,0)
			targetcamRotOffset = btQuaternion(btVector3(1, 0, 0), SIMD_PI);
		}
		else {
			// General case: Compute quaternion using cross product & dot product
			btVector3 axis = worldUp.cross(upDirection);
			if (axis.fuzzyZero()) axis = btVector3(1, 0, 0); // Fallback axis if needed
			targetcamRotOffset = btQuaternion(axis.normalized(), std::acos(worldUp.dot(upDirection)));
		}
		oldcamRotOffset = targetcamRotOffset;
		camRotOffset = targetcamRotOffset;
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

	Type getCollisionType() {
		return collisionType;
	}

	void SetOwner(Lobbies::User user) override {
		gun->SetOwner(user);
		owner.emplace(user);
	}

	void Rotate(bool positive, bool rolling, float yaw);


	void resetCollisionType() {
		collisionType = GameObject::Type::Default;
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

	inline PlayerState GetState() { return state; }
	inline void SetState(PlayerState state) { this->state = state; }

	void Damage(float amount) {
		lastHit = elapsedTime;

		health -= amount;
		if (health <= 0) {
			//state = PlayerState::DEAD;
			health = 100;
			RespawnPoint* point = Respawn::GetInstance()->GetRespawn(worldID - 1);
			GetPhysicsObject()->GetRigidBody()->getWorldTransform().setOrigin(point->position);
			setUpDirection(point->orientation);
			resetCollisionType();
			setCollided(0);
			// TODO: Create Change State packet.
		}
	}
	
	float GetMaxHealth() {
		return maxHealth;
	}
	float getCollisionJumpPadStrength(){
		return jumpPadHeight;
}

	float health = 100.0f;

	void setGun(GameObject* gunIn) { gun = gunIn; }
	GameObject* getGun() {return gun;}
	void SetGunTransform(float pitch, float yaw, btVector3 camPos);

	void UpdateObjectState() override;
	void UpdateFromState(float dt) override;

private:

	//Player Variables
	float gravityScale = 400.0f;
	float rotateTime = 0.5f;
	float maxHealth = 100.0f;

	int collided = 0;
	btVector3 collisionNormal = btVector3(0, 1, 0);
	btVector3 collisionPoint = btVector3(0, 0, 0);
	float jumpPadHeight = 0.0f;
	std::list<GameObject*> collidedObjects;
	Type collisionType;
	PlayerState state;

    btQuaternion camRotOffset;
	btQuaternion oldcamRotOffset = btQuaternion::getIdentity();
	btQuaternion targetcamRotOffset = btQuaternion::getIdentity();
	btVector3 targetWorldRotation = btVector3(0, 1, 0);
	btVector3 oldWorldRotation = btVector3(0, 1, 0);
	btVector3 upDirection;
	btVector3 rightDirection;
	btVector3 forwardDirection;
	float rotateTimer = 0.0f;
	bool rotationChanging = false;
    btVector3 gunCameraOffset = btVector3(3.0, -1.0, 1.5); // x axis is forward (front +ve/ back -ve), y is up, z is right (left -ve/ right +ve)
	GameObject* gun;


	btVector3 CalculateRightDirection(btVector3 upDir);
	btVector3 CalculateForwardDirection(btVector3 upDir, btVector3 rightDir);
	btVector3 CalculateUpDirection(float dt);
	btVector3 CalculateForwardFromYaw(float yaw);
	btVector3 CalculateRightFromYaw(float yaw);

	//float elapsedTime = 0;
	float lastHit = 0;


	GameTechRendererInterface* renderer;


};
}