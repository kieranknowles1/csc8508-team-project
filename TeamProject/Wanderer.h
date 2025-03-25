#pragma once
#include "NavEntity.h"
#include "NavMesh.h"
#include "SPGameController.h"
#include "PlayerObject.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class Wanderer : public NavEntity {
		public:
			Wanderer(PlayerObject* p, NavMesh* nav, Side side, int lID, GameTechRendererInterface* r, int l);
			~Wanderer();

			void Update(float dt);
			void InitPosAndOffset();

			void DamageAI(float d) { health -= d; }
			float GetHealth() { return health; }

			void DestroyWanderer();
			bool isDeleted() const { return deleted; }

			int laserID;

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

			PlayerObject* player;

			float maxShootTimer = 5.0f;
			float shootTimer;
			bool isShooting = true;

			float maxUpdatePlayerPathTimer = 2.0f;
			float updateplayerPathTimer;

			GameTechRendererInterface* renderer;

			int level;
			float dps;
			float health;
		};
	}
}