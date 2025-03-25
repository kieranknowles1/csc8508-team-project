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
#include "Crosshair.h"
#include "Scoreboard.h"
#include "Overheat.h"
#include <memory>


namespace NCL {
	namespace CSC8503 {

		class PlayerController {
		public:
			PlayerController(PlayerObject* playerIn, const Controller* c, Camera* cam, btDiscreteDynamicsWorld* bulletWorldIn, GameTechRendererInterface* rendererIn) {
				player = playerIn;
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

			void setYaw(float yawIn) {
				yaw = yawIn;
			}

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
			float playerSpeed = 130.0f;
			float jumpHeight = 300.0f;
			float gravityScale = 300.0f;
			float cameraHeight = 14.0f;

			float sprintMulti = 2.0f;
			float strafeMulti = 0.65f;
			float backwardsMulti = 0.55f;
			float airMulti = 0.75f;

			float slidingTime = 0.25f;
			float slidingAngle = 75.0f;
			float slidingCameraHeight = 0.0f;
			float slidingCameraBackwards = 2.5f;

			//Gun Variables
			btVector3 gunCameraOffset = btVector3(0, 0.22f, -1.5f);

			//Rotation Variables
			float rotateTime = 0.5f;


			GameTechRendererInterface* renderer;
			std::unique_ptr<Crosshair> crosshair;
			std::unique_ptr<Scoreboard> scoreboard;
			std::unique_ptr<Overheat> overheat;
			btVector3 upDirection;
			btVector3 rightDirection;
			btVector3 forwardDirection;
			btQuaternion camRotOffset;
			float rotateTimer = 0.0f;
			bool rotationChanging = false;
			bool thirdPerson = false;
			bool scoreboardActive = false;
			float spaceCount = 0;
			float inAirTime = 0;
			btDiscreteDynamicsWorld* bulletWorld;
			PlayerObject* player;
			const Controller* controller = nullptr;
			Camera* camera = nullptr;
			float yaw = 0.0f;
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
			bool firing = false;
			int laserID;
			float firingTimer = 0.0f;
			float overheatTimer = 0.0f;
			float cooldownTimer = 0.0f;

			btVector3 forward;
			btVector3 up;
			btVector3 right;
			btVector3 movement;


			Vector2 getDirectionalInput() const;
			void Initialise();
			void HandleShooting(float dt);
			void HandleSliding(float dt);
			void SpecialTypeCalculations();
			btVector3 FindFloorNormal();
			void FireShot(float dt);
			void GetAllDirections();
			void HandleYaw();
			void RotationCalculations();
			void CameraMovement();
			void GroundNormalCalculations();
			void MovementCalculations(float dt);
			void HandleJumping();
			void HandleHurtEffects();
			void ToggleScoreboard();

		};
	};


}
