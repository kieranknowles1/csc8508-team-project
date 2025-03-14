#pragma once

#include <optional>

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
#include "Paintball.h"
#include "CustomCollisionCallback.h"
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>


namespace NCL {
	namespace CSC8503 {
		class ShotInfo {
		public:
			ShotInfo(GameObject* hitObjIn, btVector3 hitPosIn, btVector3 hitNormalIn) {
				hitObj = hitObjIn; hitPos = hitPosIn; hitNormal = hitNormalIn;
			};

			GameObject* hitObj;
			btVector3 hitPos;
			btVector3 hitNormal;
		};
		class Shoot {
		public:
			Shoot() { instance = this; };
			~Shoot();
			static Shoot* GetInstance() { return instance; }
			void Initialise(btDiscreteDynamicsWorld* bulletWorldIn, ResourceManager* resourceManagerIn, GameWorld* worldIn, DecalSystem& decalSystemIn) {
				bulletWorld = bulletWorldIn; this->resourceManager = resourceManagerIn; world = worldIn; decalSystem = &decalSystemIn;

				decalTextures.push_back(resourceManager->getTextures().get(decalTexturePath));
				decalTextures.push_back(resourceManager->getTextures().get(decalTexturePath2));
				decalTextures.push_back(resourceManager->getTextures().get(decalTexturePath3));
				decalTextures.push_back(resourceManager->getTextures().get(decalTexturePath4));
			};
			void InitShotMasks(GameObject* playerIn, GameObject* gunIn) {
				player = playerIn; gun = gunIn;
			};
			std::optional<ShotInfo> RayClosest(btVector3 startPos, btVector3 dir);
			std::optional<ShotInfo> ShootBulletPlayer(btVector3 startPos, btVector3 dir,btQuaternion rotation, float dt);

		private:
			//Shot Variables
			float bulletSpeed = 2000.0f;
			btVector3 bulletCameraOffset = btVector3(1.0, -0.5, -3.0);
			btVector4 paintballColor = btVector4(1.0f, 0.0f, 0.0f, 1.0f);

			//Decal Variables
			float decalRadius = 8.0f;
			float alphaFade = 1.0f;

			std::string decalTexturePath = "paintball_splash_1.png";
			std::string decalTexturePath2 = "paintball_splash_2.png";
			std::string decalTexturePath3 = "paintball_splash_3.png";
			std::string decalTexturePath4 = "paintball_splash_4.png";

			void SpawnBulletMesh(btVector3 startPos, btVector3 dir, btQuaternion bulletRotation, ShotInfo* rayInfo);
			void SpawnDecal(ShotInfo* shotinfo);
			inline static Shoot* instance = nullptr;
			btDiscreteDynamicsWorld* bulletWorld;
			GameObject* player;
			GameObject* gun;
			ResourceManager* resourceManager;
			GameWorld* world;
			DecalSystem* decalSystem;

			std::vector<std::shared_ptr<NCL::Rendering::Texture>> decalTextures;
		};
	}
}
