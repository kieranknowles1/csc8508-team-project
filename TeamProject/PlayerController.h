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
#include "Crosshair.h"
#include "Scoreboard.h"
#include <memory>


namespace NCL {
	namespace CSC8503 {

		class PlayerController {
		public:
			PlayerController(PlayerObject* playerIn, GameObject* gunIn, const Controller* c, Camera* cam, btDiscreteDynamicsWorld* bulletWorldIn, GameTechRendererInterface* rendererIn) {
				player = playerIn;
				gun = gunIn;
				controller = c;
				camera = cam;
				bulletWorld = bulletWorldIn;
				renderer = rendererIn;


				Initialise();
			}
			~PlayerController() {};
			void UpdateMovement(float dt);
			void SetThirdPerson(bool thirdPersonIn) {
				thirdPerson = thirdPersonIn;
			};

			float getYaw() {
				return yaw;
			}

			void rollRight() {
				player->Rotate(true, true,yaw);
			}

			void rollLeft() {
				player->Rotate(false, true,yaw);
			}

			void pitchUp() {
				player->Rotate(true, false,yaw);
			}

			void pitchDown() {
				player->Rotate(false, false,yaw);
			}

		private:

			//Player Movement Variables
			float playerSpeed = 80.0f;
			float jumpHeight = 300.0f;
			float gravityScale = 300.0f;
			float cameraHeight = 4.5f;

			float sprintMulti = 2.0f;
			float strafeMulti = 0.65f;
			float backwardsMulti = 0.55f;
			float airMulti = 1.0f;

			float crouchingTime = 0.3f;
			float crouchMulti = 0.4f;
			float crouchHeight = -1.0f;

			float slidingTime = 0.25f;
			float slidingAngle = 75.0f;
			float slidingCameraHeight = 0.0f;
			float slidingCameraBackwards = 2.5f;

			//Gun Variables
			float shotCooldown = 0.075f;
			btVector3 gunCameraOffset = btVector3(1.3, -0.7, -1.2);

			//Rotation Variables
			float rotateTime = 0.5f;

			//Special Types Variables
			float bouncePadHeight = 5000.0f;


			GameTechRendererInterface* renderer;
			std::unique_ptr<Crosshair> crosshair;
			std::unique_ptr<Scoreboard> scoreboard;
			btVector3 upDirection;
			btVector3 rightDirection;
			btVector3 forwardDirection;
			btQuaternion camRotOffset;
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
			float standingHeight = 6.5f;
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

			btVector3 forward;
			btVector3 up;
			btVector3 right;
			btVector3 movement;


			Vector2 getDirectionalInput() const;
			void Initialise();
			void HandleShooting(float dt);
			void HandleCrouching(float dt);
			void HandleSliding(float dt);
			void SpecialTypeCalculations();
			bool CheckCeling();
			btVector3 FindFloorNormal();
			void SetGunTransform();
			void FireShot();
			void GetAllDirections();
			void HandleYaw();
			void RotationCalculations();
			void CameraMovement();
			void GroundNormalCalculations();
			void MovementCalculations(float dt);
			void HandleJumping();
			void HandleHurtEffects();

		};
	};


}
