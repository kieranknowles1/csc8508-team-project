#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif

#include "StateGameObject.h"
#include "PlayerController.h"
#include "Turret.h"

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

			GameObject* AddObjectToTestBulletPhysics();
			Turret* AddTurretToWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

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
			Mesh*	catMesh		= nullptr;
			Mesh*	kittenMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;

			//EG Meshes:
			Mesh* maxMesh = nullptr;
			Mesh* maleguardMesh = nullptr;
			Mesh* femaleguardMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;


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
			
			Turret* testTurret = nullptr;
		};
	}
}
