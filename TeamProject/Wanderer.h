#pragma once
#include "NavEntity.h"
#include "NavMesh.h"
#include "SPGameController.h"

namespace NCL {
	namespace CSC8503 {
		class LaserObject;
		class StateMachine;

		class Wanderer : public NavEntity {
		public:
			Wanderer(GameObject* p, NavMesh* nav, Side side, GameTechRendererInterface* r);
			~Wanderer();

			void Update(float dt);
			void InitPosAndOffset();

			void DamageAI(float d) { health -= d; }
			float GetHealth() { return health; }

			void DestroyWanderer();
			bool isDeleted() const { return deleted; }

			void SetLaser(LaserObject* laser) { this->laser = laser; }
			LaserObject* GetLaser() const { return laser; }

		private:
			void PlayerNear(float dt);
			void PlayerFar(float dt);

			NavMesh* navMesh;
			std::vector<btVector3> curPath = {};

			StateMachine* stateMachine;
			void UpdatePlayerDistance();
			float senseDistance = 100.0f;
			float playerDist = 100.0f;
			btVector3 offset;

			GameObject* player;
			LaserObject* laser;

			float maxShootTimer = 5.0f;
			float shootTimer;
			bool isShooting = true;

			float maxUpdatePlayerPathTimer = 2.0f;
			float updateplayerPathTimer;

			GameTechRendererInterface* renderer;

			float health = 50;
		};
	}
}