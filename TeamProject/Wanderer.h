#pragma once
#include "NavEntity.h"
#include "NavMesh.h"
#include "SPGameController.h"
#include "PlayerObject.h"

namespace NCL {
	namespace CSC8503 {
		class LaserObject;
		class StateMachine;
		class HealthAttrib;
		class AttackAttrib;

		class Wanderer : public NavEntity {
		public:
			Wanderer(PlayerObject* p, NavMesh* nav, Side side, GameTechRendererInterface* r, int difficulty);
			~Wanderer();

			void Update(float dt);
			void InitPosAndOffset();

			void DestroyWanderer();
			bool isDeleted() const { return deleted; }

			void SetLaser(LaserObject* laser) { this->laser = laser; }
			LaserObject* GetLaser() const { return laser; }

			HealthAttrib* GetHealthAttrib() { return health.get(); }
			AttackAttrib* GetAttackAttrib() { return attack.get(); }

		private:
			std::unique_ptr<HealthAttrib> health = nullptr;
			std::unique_ptr<AttackAttrib> attack = nullptr;

			void PlayerNear(float dt);
			void PlayerFar(float dt);

			NavMesh* navMesh;
			std::vector<btVector3> curPath = {};

			StateMachine* stateMachine;
			void UpdatePlayerDistance();
			float senseDistance = 250.0f;
			float playerDist = 250.0f;
			btVector3 offset;

			PlayerObject* player;
			LaserObject* laser;

			float maxShootTimer = 5.0f;
			float shootTimer;
			bool isShooting = true;

			float maxUpdatePlayerPathTimer = 2.0f;
			float updateplayerPathTimer;

			GameTechRendererInterface* renderer;

			int difficulty = 1;
		};
	}
}