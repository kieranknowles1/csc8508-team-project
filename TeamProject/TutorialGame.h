#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif

#include "LevelImporter.h"
#include "StateGameObject.h"
#include "PlayerController.h"
#include "PlayerObject.h"
#include "Turret.h"
#include "NavMesh.h"

#include <btBulletDynamicsCommon.h>

namespace NCL {
	namespace CSC8503 {
		class BulletDebug;

		class TutorialGame {
		public:
			// Physics update frequency, in hertz
			const static constexpr float PHYSICS_PERIOD = 1.0f / 60.0f;


			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();
			void ThirdPersonControls();
			void InitWorld();

			Turret* AddTurretToWorld();

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size, const Vector3& rotation, bool isFloor);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f,bool hasCollision = true);
			PlayerObject* AddPlayerCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);
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
			//Added Shaders:
			Shader* flatShader = nullptr;

			//Coursework Meshes
			Mesh*	catMesh		= nullptr;
			Mesh*	kittenMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;

			//EG Meshes:
			Mesh* maxMesh = nullptr;
			Mesh* maleguardMesh = nullptr;
			Mesh* femaleguardMesh = nullptr;

			//EG Level Mehses:
			Mesh* wallSection = nullptr;
			Mesh* floorSection = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;


			/* bullet physics stuff here */
			btDiscreteDynamicsWorld* bulletWorld = nullptr;
			btBroadphaseInterface* broadphase = nullptr;
			btDefaultCollisionConfiguration* collisionConfig = nullptr;
			btCollisionDispatcher* dispatcher = nullptr;
			btSequentialImpulseConstraintSolver* solver = nullptr;

			BulletDebug* bulletDebug = nullptr;

			void DestroyBullet();
			void InitBullet(); // Initialises the Bullet physics world

			//Player things
			void InitPlayer();
			PerspectiveCamera* mainCamera;
			PlayerObject* player;
			GameObject* gun;
			PlayerController* playerController;
			bool freeCam = false;
			bool thirdPerson = false;
			Vector4 playerColour = Vector4(1, 0.8, 1, 1);

			//fixed update 
			float accumulator = 0.0f;
			float fixedDeltaTime = 1.0f / 60.0f;

			//AI
			Turret* testTurret = nullptr;

			//Level import
			LevelImporter* levelImporter;
			bool loadFromLevel;

			NavMesh* navMesh;
		};
	}
}
