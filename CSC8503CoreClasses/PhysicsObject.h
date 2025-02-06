#pragma once

#include <btBulletDynamicsCommon.h>

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		// Wrapper to apply Bullet physics to an object. Holds the transform therefore is required for all objects
		// NOTE: Bullet deactivates rigid bodies after inactivity, when applying forces they must be manually be activated
		// using GetRigidBody()->activate()
		class PhysicsObject	{
		public:
			PhysicsObject(GameObject* parent);
			~PhysicsObject();

			// Add Bullet-specific methods
			void InitBulletPhysics(btDynamicsWorld* world, btCollisionShape* shape, float mass);
			btRigidBody* GetRigidBody() { return rigidBody; }

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);
			
			void AddForce(const Vector3& force);
			void AddForceAtPosition(const Vector3& force, const Vector3& position);
			void AddTorque(const Vector3& torque);

			btMotionState* GetMotionState() const {
				return motionState;
			}

			void ClearForces();

		protected:
			GameObject* parent;

			// bullet stuff
			btRigidBody* rigidBody;
			btMotionState* motionState;
			btCollisionShape* collisionShape;
		};
	}
}
