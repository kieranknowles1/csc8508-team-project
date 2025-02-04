#pragma once

#include <btBulletDynamicsCommon.h>

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class PhysicsObject	{
		public:
			PhysicsObject(GameObject* parent);
			~PhysicsObject();

			// Add Bullet-specific methods
			void InitBulletPhysics(btDynamicsWorld* world, btCollisionShape* shape, float mass);
			void UpdateFromBullet();
			btRigidBody* GetRigidBody() { return rigidBody; }

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);
			
			void AddForce(const Vector3& force);
			void AddForceAtPosition(const Vector3& force, const Vector3& position);
			void AddTorque(const Vector3& torque);

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

