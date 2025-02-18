#pragma once

#include "ResourceManager.h"
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "BulletDebug.h"
#include "PlayerObject.h"
#include "CustomCollisionCallback.h"

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>


namespace NCL {
	namespace CSC8503 {
		class PlayerController {
		public:
			PlayerController(PlayerObject* playerIn, GameObject* gunIn, const Controller& c, Camera* cam, btDiscreteDynamicsWorld* bulletWorldIn, GameWorld* worldIn, ResourceManager* resourceManager) {
				player = playerIn;
				gun = gunIn;
				controller = &c;
				camera = cam;
				bulletWorld = bulletWorldIn;
				world = worldIn;
				this->resourceManager = resourceManager;
				Initialise();
			}
			~PlayerController() {};
			void UpdateMovement(float dt);
			void SetThirdPerson(bool thirdPersonIn) {
				thirdPerson = thirdPersonIn;
			};
			void setTargetWorldRotation(btVector3 worldRotationIn) {
				if (rotationChanging) return;
				oldWorldRotation = upDirection;
				targetWorldRotation = worldRotationIn;
				rotateTimer = 0.0f;
				rotationChanging = true;
			}
			btVector3 getOldWorldRotation() {
				return oldWorldRotation;
			}
			btVector3 getUpDirection() {
				return upDirection;
			}
			btVector3 getRightDirection() {
				return rightDirection;
			}
			btVector3 getForwardDirection() {
				return forwardDirection;
			}

			void CalculateDirections(float dt);
			btVector3 CalculateRightDirection(btVector3 upDir);
			btVector3 CalculateForwardDirection(btVector3 upDir, btVector3 rightDir);

		private:

			btVector3 targetWorldRotation = btVector3(0, 1, 0);
			btVector3 oldWorldRotation = btVector3(0,1,0);

			btVector3 upDirection;
			btVector3 rightDirection;
			btVector3 forwardDirection;

			//Player Movement Variables
			float playerSpeed = 60.0f;
			float jumpHeight = 75.0f;
			float gravityScale = 100.0f;
			float cameraHeight = 3.0f;
			float airMulti = 1.0f;
			float strafeMulti = 0.65f;
			float backwardsMulti = 0.55f;
			float sprintMulti = 2.0f;
			float crouchingTime = 0.3f;
			float crouchMulti = 0.4f;
			float crouchHeight = 0.0f;
			float slidingTime = 0.25f;
			float jumpDampening = 0.2f;
			float slidingDampening = 0.2f;
			float slidingFriction = 0.25f;
			float floorDampening = 0.2f;
			float slidingAngle = 75.0f;
			float slidingCameraHeight = 0.0f;
			float slidingCameraBackwards = 2.5f;

			//Gun Variables
			float shotCooldown = 0.25f;
			float bulletSpeed = 500.0f;
			btVector3 gunCameraOffset = btVector3(1.3, -0.7, -1.2);
			btVector3 bulletCameraOffset = btVector3(1.0, -0.5, -3.0);
			float playerVelocityStrafeInherit = 0.05f;

			//Rotation Variables
			float rotateTime = 0.5f;

			float rotateTimer = 0.0f;
			bool rotationChanging = false;
			bool thirdPerson = false;
			float spaceCount = 0;
			float inAirTime = 0;
			btDiscreteDynamicsWorld* bulletWorld;
			PlayerObject* player;
			GameObject* gun;
			const Controller* controller = nullptr;
			Camera* camera = nullptr;
			float yaw = 0;
			float roll = 0;
			float radius = 2.0f;
			bool crouchTransition = false;
			float currentHeight;
			float standingHeight = 4.0f;
			float crouchingHeight = 2.0f;
			float currentCrouchingTimer=0;
			float currentStandingTimer=10.0f;
			bool isCrouching;
			bool isSliding = false;
			bool slideTransition = false;
			float currentAngle;
			float crouchingAngle = 2.0f;
			float currentSlidingTimer = 0;
			float currentStandingSlideTimer = 10.0f;
			btRigidBody* rb;
			btTransform transformPlayer;
			btVector3 btPlayerPos;
			btTransform transformGun;
			btVector3 btGunPos;
			ResourceManager* resourceManager;
			GameWorld* world;
			float shotTimer = 0;
			bool collision = false;
			bool crouching = false;


			Vector2 getDirectionalInput() const;
			void Initialise();
			void HandleCrouching(float dt);
			void HandleSliding(float dt);
			bool CheckCeling();
			btVector3 FindFloorNormal();
			void SetGunTransform();
			void ShootBullet();
			btVector3 CalculateUpDirection(float dt);
			
			float CalculateRoll();

		};
	};
}


using namespace NCL::CSC8503;
// Paintball class derived from GameObject
class Paintball : public GameObject {
public:
	void OnCollisionEnter(const CollisionInfo& collisionInfo) override {
		if (collisionInfo.otherObject == player) return;
		collisionInfo.otherObject->GetRenderObject()->SetColour(this->GetRenderObject()->GetColour());
		collisionInfo.otherObject->GetRenderObject()->SetIsFlat(true);
		player->GetRenderObject()->SetColour(this->GetRenderObject()->GetColour());
		player->GetRenderObject()->SetIsFlat(true);
		this->GetPhysicsObject()->removeFromBullet(bulletWorld);
	}
	void Initialise(GameObject* playerIn, btDiscreteDynamicsWorld* bulletWorldIn) {
		player = playerIn;
		bulletWorld = bulletWorldIn;
	}

private:
	GameObject* player;
	btDiscreteDynamicsWorld* bulletWorld;
};




