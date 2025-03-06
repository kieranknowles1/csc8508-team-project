#pragma once
#include "NavEntity.h"
#include "NavMesh.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class Wanderer : public NavEntity {
		public:
			Wanderer(GameObject* p, NavMesh* nav, GameObject* g);
			~Wanderer();

			void Update(float dt);
			void InitPosAndOffset();

		private:
			void PlayerNear();
			void PlayerFar();
			void SetGunTransform();

			NavMesh* navMesh;
			std::vector<btVector3> curPath = {};

			StateMachine* stateMachine;
			void UpdatePlayerDistance();
			float senseDistance = 100.0f;
			float playerDist = 100.0f;
			btVector3 offset;

			GameObject* player;
			GameObject* gun;
		};
	}
}