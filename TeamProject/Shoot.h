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
#include "CustomCollisionCallback.h"
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>


namespace NCL {
	namespace CSC8503 {
		class Shoot {
		public:
			Shoot() { instance = this; }
			static Shoot* GetInstance(){ return instance; }
			~Shoot();

			GameObject* ShootBullet(btVector3 startPos, btVector3 dir);
		private:
			inline static Shoot* instance = nullptr;
		};
	}
}