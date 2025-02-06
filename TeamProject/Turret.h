#pragma once
#include "GameObject.h"

#include <LinearMath/btQuaternion.h>

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class Turret : public GameObject {
		public:
			Turret(btQuaternion q = btQuaternion());
			~Turret();

			virtual void Update(float dt);

		protected:
			void RotateLeft(float dt);
			void RotateRight(float dt);

			StateMachine* stateMachine;
			btQuaternion yPositive;
			btQuaternion yNegative;
			float rotateTime;
			float rotateSpeed;
		};
	}
}