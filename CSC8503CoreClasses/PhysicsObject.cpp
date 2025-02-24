#include "PhysicsObject.h"
#include "GameObject.h"

#include <btBulletDynamicsCommon.h>

namespace NCL::CSC8503 {

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

void PhysicsObject::ClearForces() {
	if (rigidBody) {
		rigidBody->clearForces();
	}
}

void PhysicsObject::sendCollisionEvents(const CollisionInfo& info) {
	if (!lastFrameCollisions.contains(info.otherObject)) {
		parent->OnCollisionEnter(info);
	}
	else {
		 parent->OnCollisionStay(info);
	}
	thisFrameCollisions.insert(info.otherObject);
}

void PhysicsObject::Update() {
	 for (auto it : lastFrameCollisions) {
	 	if (!thisFrameCollisions.contains(it)) {
	 		parent->OnCollisionExit(it);
	 	}
	 }
	std::swap(thisFrameCollisions, lastFrameCollisions);
	thisFrameCollisions.clear();
}

}
