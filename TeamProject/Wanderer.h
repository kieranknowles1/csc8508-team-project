#pragma once
#include "NavEntity.h"
#include "NavMesh.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class Wanderer : public NavEntity {
		public:
			Wanderer(GameObject* p, NavMesh* nav, char side, int lID);
			~Wanderer();

			void Update(float dt);
			void InitPosAndOffset();

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

			float maxShootTimer = 5.0f;
			float shootTimer;

			float maxUpdatePlayerPathTimer = 2.0f;
			float updateplayerPathTimer;

			int laserID;
		};
	}
}