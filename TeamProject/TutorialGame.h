#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif

#include "StateGameObject.h"
#include "PlayerController.h"

#include <btBulletDynamicsCommon.h>

namespace NCL {
	namespace CSC8503 {
		class BulletDebug;

		class TutorialGame {
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void InitDefaultFloor();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

			GameObject* AddInfinitePlaneToWorld(const Vector3& position, const Vector3& normal, float planeConstant);


#ifdef USEVULKAN
			GameTechVulkanRenderer* renderer;
#else
			GameTechRenderer* renderer;
#endif
			GameWorld* world;

			KeyboardMouseController controller;


			Mesh* planeMesh = nullptr;
			Mesh* capsuleMesh = nullptr;
			Mesh* cubeMesh = nullptr;
			Mesh* sphereMesh = nullptr;

			Texture* basicTex = nullptr;
			Shader* basicShader = nullptr;

			//Coursework Meshes
			Mesh* catMesh = nullptr;
			Mesh* kittenMesh = nullptr;
			Mesh* enemyMesh = nullptr;
			Mesh* bonusMesh = nullptr;

			/* bullet physics stuff here */
			btDiscreteDynamicsWorld* bulletWorld;
			btBroadphaseInterface* broadphase;
			btDefaultCollisionConfiguration* collisionConfig;
			btCollisionDispatcher* dispatcher;
			btSequentialImpulseConstraintSolver* solver;

			BulletDebug* bulletDebug;

			void InitBullet(); // Initialises the Bullet physics world
			GameObject* objectToTestBulletPhysics = nullptr;

			//Player things
			void InitPlayer();
			PerspectiveCamera* mainCamera;
			GameObject* player;
			PlayerController* playerController;
			bool freeCam = false;

			//fixed update 
			float accumulator = 0.0f;
			float fixedDeltaTime = 1.0f / 60.0f;
			void FixedUpdate();
		};
	}
}
