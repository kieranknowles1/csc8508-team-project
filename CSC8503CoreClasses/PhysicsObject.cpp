#include "PhysicsObject.h"
#include "GameObject.h"

#include "CustomCollisionCallback.h"

#include <btBulletDynamicsCommon.h>

using namespace NCL;
using namespace CSC8503;

PhysicsObject::PhysicsObject(GameObject* parent)
	: parent(parent), rigidBody(nullptr), motionState(nullptr) {

}

PhysicsObject::~PhysicsObject()	{
	// Clean up Bullet physics components
	if (rigidBody) {
		delete rigidBody->getMotionState();
		delete rigidBody->getCollisionShape();
		delete rigidBody;
	}

#ifndef NDEBUG
	if (hasBullet) {
		std::cerr << "WARN: PhysicsObject was not removed from Bullet world, this will cause use-after-free" << std::endl;
	}
#endif
}

/* Bullet Physics Implementation start here */
void PhysicsObject::InitBulletPhysics(btDynamicsWorld* world, btCollisionShape* shape, float mass, bool collide) {

	btTransform startTransform;
	startTransform.setIdentity();

	// Setting the starting position of the object using the NCL framework's transform
	startTransform.setOrigin(parent->getInitialPosition());
	startTransform.setRotation(parent->getInitialRotation());

	// MotionState has been used to retrieve and apply Bullet's physics transformations to the NCL object
	motionState = new btDefaultMotionState(startTransform);

	btVector3 localInertia(0, 0, 0);
	if (mass > 0.0f && shape) {
		shape->calculateLocalInertia(mass, localInertia);
	}

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
	rigidBody = new btRigidBody(rbInfo);

	// Setting the object's properties
	rigidBody->setMassProps(mass, localInertia);
	rigidBody->setUserPointer(parent);
	rigidBody->setActivationState(DISABLE_DEACTIVATION);

	if (collide) {
		world->addRigidBody(rigidBody);
#ifndef NDEBUG
		hasBullet = true;
#endif
	}
}

void NCL::CSC8503::PhysicsObject::removeFromBullet(btDynamicsWorld* world)
{
	if (rigidBody) {
		world->removeRigidBody(rigidBody);
#ifndef NDEBUG
		hasBullet = false;
#endif
	}

}

void PhysicsObject::AddForce(const Vector3& force) {
	if (rigidBody) {
		rigidBody->applyCentralForce(btVector3(force.x, force.y, force.z));
	}
}

void PhysicsObject::AddForceAtPosition(const Vector3& force, const Vector3& position) {
	if (rigidBody) {
		rigidBody->applyForce(btVector3(force.x, force.y, force.z), btVector3(position.x, position.y, position.z));
	}
}

void PhysicsObject::AddTorque(const Vector3& torque) {
	if (rigidBody) {
		rigidBody->applyTorque(btVector3(torque.x, torque.y, torque.z));
	}
}

void PhysicsObject::ApplyAngularImpulse(const Vector3& impulse) {
	if (rigidBody) {
		rigidBody->applyTorqueImpulse(btVector3(impulse.x, impulse.y, impulse.z));
	}
}

void PhysicsObject::ApplyLinearImpulse(const Vector3& impulse) {
	if (rigidBody) {
		rigidBody->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
	}
}

void PhysicsObject::CheckCollisions(btDynamicsWorld* world)
{
	if (!rigidBody || !world) {
		return;
	}

	// Collect a list of all objects we are currently colliding with
	// Send out events for new collisions and ended collisions
	CustomCollisionCallback callback(parent);
	world->contactTest(rigidBody, callback);

	// New collisions
	for (auto obj : callback.activeCollisions) {
		if (!activeCollisions.count(obj)) {
			activeCollisions.insert(obj);
			parent->OnCollisionEnter(obj);
		}
	}

	// Ended collisions
	for (auto obj : activeCollisions) {
		if (!callback.activeCollisions.count(obj)) {
			parent->OnCollisionExit(obj);
		}
	}

	// Can't erase from a set while iterating over it
	std::erase_if(activeCollisions, [&](GameObject* obj) {
		return !callback.activeCollisions.count(obj);
	});
}

void PhysicsObject::ClearForces() {
	if (rigidBody) {
		rigidBody->clearForces();
	}
}
