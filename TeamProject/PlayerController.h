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
#include "DecalSystem.h"
#include "Shoot.h"
#include "Paintball.h"


namespace NCL {
	namespace CSC8503 {

		class PlayerController {
		public:
			PlayerController(PlayerObject* playerIn, GameObject* gunIn, const Controller* c, Camera* cam, btDiscreteDynamicsWorld* bulletWorldIn) {
				player = playerIn;
				gun = gunIn;
				controller = c;
				camera = cam;
				bulletWorld = bulletWorldIn;
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

			btVector3 getUpDirection() {
				return upDirection;
			}

			btVector3 getRightDirection() {
				return rightDirection;
			}

			btVector3 getForwardDirection() {
				return forwardDirection;
			}

			float getYaw() {
				return yaw;
			}
			btQuaternion getCamOffset() {
				return camRotOffset;
			}

			void rollRight() {
				Rotate(true, true);
			}

			void rollLeft() {
				Rotate(false, true);
			}

			void pitchUp() {
				Rotate(true, false);
			}

			void pitchDown() {
				Rotate(false, false);
			}

			void CalculateDirections(float dt);
			btVector3 CalculateRightDirection(btVector3 upDir);
			btVector3 CalculateForwardDirection(btVector3 upDir, btVector3 rightDir);

		private:

			//Player Movement Variables
			float playerSpeed = 80.0f;
			float jumpHeight = 300.0f;
			float gravityScale = 300.0f;
			float cameraHeight = 3.0f;

			float sprintMulti = 2.0f;
			float strafeMulti = 0.65f;
			float backwardsMulti = 0.55f;
			float airMulti = 1.0f;

			float crouchingTime = 0.3f;
			float crouchMulti = 0.4f;
			float crouchHeight = 0.0f;

			float slidingTime = 0.25f;
			float slidingAngle = 75.0f;
			float slidingCameraHeight = 0.0f;
			float slidingCameraBackwards = 2.5f;

			//Gun Variables
			float shotCooldown = 0.075f;
			float bulletSpeed = 1000.0f;
			btVector3 gunCameraOffset = btVector3(1.3, -0.7, -1.2);

			//Rotation Variables
			float rotateTime = 0.5f;

			//Special Types Variables
			float bouncePadHeight = 5000.0f;

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
			bool thirdPerson = false;
			float spaceCount = 0;
			float inAirTime = 0;
			btDiscreteDynamicsWorld* bulletWorld;
			PlayerObject* player;
			GameObject* gun;
			const Controller* controller = nullptr;
			Camera* camera = nullptr;
			float yaw = 0;
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
			GameWorld* world;
			float shotTimer = 0;
			bool collision = false;
			bool crouching = false;
			bool rollUse = false;
			btIDebugDraw* debugDrawer;
			bool onIce = false;
			btVector3 previousVelocity;

			Vector2 getDirectionalInput() const;
			void Initialise();
			void HandleShooting(float dt);
			void HandleCrouching(float dt);
			void HandleSliding(float dt);
			void HandleTypes();
			bool CheckCeling();
			btVector3 FindFloorNormal();
			void SetGunTransform();
			void FireShot();
			void Rotate(bool positive, bool rolling);
			btVector3 CalculateUpDirection(float dt);
			btVector3 CalculateForwardFromYaw();
			btVector3 CalculateRightFromYaw();
		};
	};


}
