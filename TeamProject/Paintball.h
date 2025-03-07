#pragma once

#include "CustomCollisionCallback.h"
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>


namespace NCL {
	namespace CSC8503 {
		// Paintball class derived from GameObject
		class Paintball : public GameObject {
		public:
			void OnCollisionEnter(const CollisionInfo& collisionInfo) override;
			void Initialise(GameObject* playerIn) {
				player = playerIn;
			}
		private:
			GameObject* player;
		};
	}
}