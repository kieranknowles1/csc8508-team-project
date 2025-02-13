#pragma once
#include "GameObject.h"

#include <LinearMath/btQuaternion.h>

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class Turret : public GameObject {
		public:
			Turret(GameObject* p, btQuaternion q = btQuaternion());
			~Turret() override;

			void Update(float dt) override;

		protected:
			void RotateLeft(float dt);
			void RotateRight(float dt);

			StateMachine* stateMachine;
			btQuaternion yPositive;
			btQuaternion yNegative;
			float rotateTime;
			float rotateSpeed;

			GameObject* player;
			btTransform trans;
		};
	}
}
