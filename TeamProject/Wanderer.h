#pragma once
#include "NavEntity.h"
#include "NavMesh.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class Wanderer : public NavEntity {
		public:
			Wanderer(GameObject* p, NavMesh* nav);
			~Wanderer();

			void Update(float dt);
			//void SetOffset();

		private:
			void PlayerNear();
			void PlayerFar();

			NavMesh* navMesh;
			std::vector<btVector3> curPath = {};

			StateMachine* stateMachine;
			void canSeePlayer();
			float senseDistance = 100.0f;
			float playerDist = 100.0f;

			GameObject* player;
		};
	}
}