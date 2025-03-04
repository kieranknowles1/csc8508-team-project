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
				pngTexture = resourceManager->getTextures().get(decalTexturePath);
			};
			void InitShotMasks(GameObject* playerIn, GameObject* gunIn) {
				player = playerIn; gun = gunIn;
			};
			ShotInfo* RayClosest(btVector3 startPos, btVector3 dir);
			ShotInfo* ShootBulletPlayer(btVector3 startPos, btVector3 dir,btQuaternion rotation);

		private:
			//Shot Variables
			float bulletSpeed = 1000.0f;
			btVector3 bulletCameraOffset = btVector3(1.0, -0.5, -3.0);

			//Decal Variables
			float decalRadius = 8.0f;
			float alphaFade = 1.0f;
			btVector4 decalColor = btVector4(1.0f, 0.0f, 0.0f, 1.0f);
			std::string decalTexturePath = "paintball_splash_red.png";


			void SpawnBulletMesh(btVector3 startPos, btVector3 dir, btQuaternion bulletRotation, ShotInfo* rayInfo);
			void SpawnDecal(ShotInfo* shotinfo);
			inline static Shoot* instance = nullptr;
			btDiscreteDynamicsWorld* bulletWorld;
			GameObject* player;
			GameObject* gun;
			ResourceManager* resourceManager;
			GameWorld* world;
			DecalSystem* decalSystem;
			std::shared_ptr<NCL::Rendering::Texture> pngTexture = nullptr;
		};
	}
}