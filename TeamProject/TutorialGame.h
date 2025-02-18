#pragma once

#include "../NCLCoreClasses/KeyboardMouseController.h"

#include "GameTechRendererInterface.h"

#include "ResourceManager.h"

#include "LevelImporter.h"
#include "PlayerController.h"
#include "PlayerObject.h"
#include "Turret.h"
#include "NavMesh.h"
#include "Profiler.h"


#include <btBulletDynamicsCommon.h>

namespace NCL {
	namespace CSC8503 {
		class BulletDebug;

		class TutorialGame {
		public:
			// Physics update frequency, in hertz
			const static constexpr float PHYSICS_PERIOD = 1.0f / 60.0f;


			TutorialGame(GameTechRendererInterface* renderer, GameWorld* world, Controller* controller);
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();
			void ThirdPersonControls();
			void InitWorld();


			void UpdatePlayer(float dt);


			Turret* AddTurretToWorld();

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size, const Vector3& rotation, bool isFloor);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f,bool hasCollision = true);
			PlayerObject* AddPlayerCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

			GameObject* AddInfinitePlaneToWorld(const Vector3& position, const Vector3& normal, float planeConstant);

			std::unique_ptr<ResourceManager> resourceManager;
			bool showProfiling = false;
			Profiler profiler;

			GameTechRendererInterface* renderer;
			GameWorld* world;

			Controller* controller;

			std::shared_ptr<Texture> defaultTexture;
			std::shared_ptr<Shader> defaultShader;

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

			void CheckCollisions();
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


		};
	}
}
