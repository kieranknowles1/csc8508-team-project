#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>


namespace NCL {
	namespace CSC8503 {
		class PlayerController {
		public:
			PlayerController(GameObject* playerIn, const Controller& c, Camera* cam, btDiscreteDynamicsWorld* bulletWorldIn) {
				player = playerIn;
				controller = &c;
				camera = cam;
				bulletWorld = bulletWorldIn;
				rb = player->GetPhysicsObject()->GetRigidBody();
			}
			~PlayerController();
			void UpdateMovement(float dt);
			void SetThirdPerson(bool thirdPersonIn) {
				thirdPerson = thirdPersonIn;
			};
		private:
			float playerSpeed = 6000.0f;
			float jumpHeight = 4000.0f;
			float maxJumpTime = 0.2f;
			float gravityScale = 2000.0f;
			float cameraHeight = 3.0f;
			float diagonalMulti = 0.6f;
			float strafeMulti = 0.65f;
			float backwardsMulti = 0.55f;
			float sprintMulti = 2.0f;
			float crouchingTime = 0.3f;
			float crouchMulti = 0.4f;
			float crouchHeight = 0.0f;
			float slidingTime = 0.25f;
			float slidingDampening = 0.45f;
			float normalDampening = 0.999f;
			float slidingAngle = 70.0f;
			float slidingCameraHeight = 0.0f;
			float slidingCameraBackwards = 2.5f;

			bool thirdPerson = false;
			bool inAir = false;
			float spaceCount = 0;
			float inAirCount = 0;
			btDiscreteDynamicsWorld* bulletWorld;
			GameObject* player;
			const Controller* controller = nullptr;
			Camera* camera = nullptr;
			float yaw = 0;
			float radius = 2.0f;
			bool crouchTransition = false;
			float currentHeight;
			float standingHeight = 4.0f;
			float crouchingHeight = 2.0f;
			float currentCrouchingTimer=0;
			float currentStandingTimer=10.0f;
			bool isCrouching;
			bool isSliding;
			bool slideTransition = false;
			float currentAngle;
			float crouchingAngle = 2.0f;
			float currentSlidingTimer = 0;
			float currentStandingSlideTimer = 10.0f;
			btRigidBody* rb;
			btTransform transformPlayer;
			btVector3 btPlayerPos;

			void HandleCrouching(float dt);
			void HandleSliding(float dt);
			void CheckFloor(float dt);

		};
	};
}