#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"
#include "StateGameObject.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "BulletDebug.h"

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>


namespace NCL {
	namespace CSC8503 {
		class PlayerController {
		public:
			PlayerController(GameObject* playerIn, GameObject* gunIn, const Controller& c, Camera* cam, btDiscreteDynamicsWorld* bulletWorldIn, GameWorld* worldIn ,GameTechRenderer* rendererIn) {
				player = playerIn;
				gun = gunIn;
				controller = &c;
				camera = cam;
				bulletWorld = bulletWorldIn;
				world = worldIn;
			//	renderer = rendererIn;
				Initialise();
			}
			~PlayerController() {};


			void UpdateMovement(float dt);
			void SetThirdPerson(bool thirdPersonIn) {
				thirdPerson = thirdPersonIn;
			};
		private:
			//Player Movement Variables
			float playerSpeed = 6000.0f;
			float jumpHeight = 700.0f;
			float maxJumpTime = 0.0f;
			float gravityScale = 1000.0f;
			float cameraHeight = 3.0f;
			float airMulti = 0.1f;
			float diagonalMulti = 0.6f;
			float strafeMulti = 0.65f;
			float backwardsMulti = 0.55f;
			float sprintMulti = 2.0f;
			float crouchingTime = 0.3f;
			float crouchMulti = 0.4f;
			float crouchHeight = 0.0f;
			float slidingTime = 0.25f;
			float jumpDampening = 0.2f;
			float slidingDampening = 0.2f;
			float floorDampening = 0.999f;
			float slidingAngle = 75.0f;
			float slidingCameraHeight = 0.0f;
			float slidingCameraBackwards = 2.5f;

			//Gun Variables
			float shotCooldown = 0.25f;
			float bulletSpeed = 150.0f;
			btVector3 gunCameraOffset = btVector3(1.3, -0.7, -1.2);
			btVector3 bulletCameraOffset = btVector3(1.0, -0.5, -3.0);
			float playerVelocityStrafeInherit = 0.2f;
		

			bool thirdPerson = false;
			bool inAir = false;
			float spaceCount = 0;
			float inAirCount = 0;
			btDiscreteDynamicsWorld* bulletWorld;
			GameObject* player;
			GameObject* gun;
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
			btTransform transformGun;
			btVector3 btGunPos;
			Mesh* sphereMesh = nullptr;
			Texture* basicTex = nullptr;
			Shader* basicShader = nullptr;
			GameWorld* world;
			GameTechRenderer* renderer;
			float shotTimer = 0;

			void Initialise();
			void HandleCrouching(float dt);
			void HandleSliding(float dt);
			void CheckFloor(float dt);
			void SetGunTransform();
			void ShootBullet();

		};
	};
}


using namespace NCL::CSC8503;
// Bullet class derived from GameObject
class Bullet : public GameObject {
public:
	void OnCollisionEnter(GameObject* otherObject) override {
		if (otherObject == player) return;
		otherObject->GetRenderObject()->SetColour(this->GetRenderObject()->GetColour());
		btTransform worldTransform;
		worldTransform.setOrigin(btVector3(0, -100, 0));
		this->GetPhysicsObject()->removeFromBullet(bulletWorld);
		this->GetPhysicsObject()->GetRigidBody()->setWorldTransform(worldTransform);
		this->GetRenderObject()->SetColour(Vector4(1, 1, 1, 0));
	}
	void Initialise(GameObject* playerIn, btDiscreteDynamicsWorld* bulletWorldIn) {
		player = playerIn;
		bulletWorld = bulletWorldIn;
	}

private:
	GameObject* player;
	btDiscreteDynamicsWorld* bulletWorld;
};

