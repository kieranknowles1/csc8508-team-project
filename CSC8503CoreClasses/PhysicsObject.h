#pragma once

#include <btBulletDynamicsCommon.h>
#include "Maths.h"

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class CustomCollisionCallback;

		// Wrapper to apply Bullet physics to an object. Holds the transform therefore is required for all objects
		// NOTE: Bullet deactivates rigid bodies after inactivity, when applying forces they must be manually be activated
		// using GetRigidBody()->activate()
		class PhysicsObject	{
		public:
			// https://stackoverflow.com/questions/14243854/c-dynamic-cast-causes-a-segfault-even-when-the-object-that-is-casted-is-not-n
			// Bullet is built without RTTI on MSVC, so holding type in a user index
			// Index defaults to -1 (all 1s), using a flag would need it to be set everywhere
			enum class ShapeType {
				Compound = 1
			};

			PhysicsObject(GameObject* parent);
			~PhysicsObject();

			// Add Bullet-specific methods
			void InitBulletPhysics(btDynamicsWorld* world, btCollisionShape* shape, float mass, bool collide=true);
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
		};
	}
}
