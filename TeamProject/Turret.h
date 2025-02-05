#pragma once
#include "GameObject.h"
#include "PhysicsSystem.h"
#include "Quaternion.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class Turret : public GameObject {
		public:
			Turret(Quaternion q = Quaternion());
			~Turret();

			virtual void Update(float dt);
			void setInitialRotation(Quaternion q);

		protected:
			void RotateLeft(float dt);
			void RotateRight(float dt);

			StateMachine* stateMachine;
			Quaternion initialRotation;
			Quaternion yPositive;
			Quaternion yNegative;
			float rotateTime;
			float rotateSpeed;
		};
	}
}