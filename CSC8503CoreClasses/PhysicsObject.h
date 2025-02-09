#pragma once

#include <btBulletDynamicsCommon.h>
#include "Maths.h"

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class CustomCollisionCallback;

		class PhysicsObject	{
		public:
			PhysicsObject(GameObject* parent);
			~PhysicsObject();

			// Add Bullet-specific methods
			void InitBulletPhysics(btDynamicsWorld* world, btCollisionShape* shape, float mass, bool collide=true);
			void removeFromBullet(btDynamicsWorld* world);
			btRigidBody* GetRigidBody() { return rigidBody; }

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);

			void AddForce(const Vector3& force);
			void AddForceAtPosition(const Vector3& force, const Vector3& position);
			void AddTorque(const Vector3& torque);

			btMotionState* GetMotionState() const {
				return motionState;
			}

			// Checks the object's currently in collision
			void CheckCollisions(btDynamicsWorld* world);

			void ClearForces();

		protected:
			GameObject* parent;

			// bullet stuff
			btRigidBody* rigidBody;
			btMotionState* motionState;

			std::set<GameObject*> activeCollisions;

#ifndef NDEBUG
			bool hasBullet = false;
#endif
		};
	}
}
