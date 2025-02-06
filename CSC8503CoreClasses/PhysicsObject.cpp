#include "PhysicsObject.h"
#include "GameObject.h"
#include "Transform.h"

#include <btBulletDynamicsCommon.h>

using namespace NCL;
using namespace CSC8503;

PhysicsObject::PhysicsObject(GameObject* parent) 
	: parent(parent), rigidBody(nullptr), motionState(nullptr), collisionShape(nullptr) {
	
}

PhysicsObject::~PhysicsObject()	{
	// Clean up Bullet physics components
	if (rigidBody) {
		delete rigidBody->getMotionState();
		delete rigidBody;
	}
	if (collisionShape) {
		delete collisionShape;
	}
}

/* Bullet Physics Implementation start here */
void PhysicsObject::InitBulletPhysics(btDynamicsWorld* world, btCollisionShape* shape, float mass, bool collide) {
	collisionShape = shape;
	btTransform startTransform;
	startTransform.setIdentity();

	Vector3 initialPosition = parent->getInitialPosition();
	Quaternion initialOrientation = parent->getInitialRotation();

	// Setting the starting position of the object using the NCL framework's transform
	startTransform.setOrigin(btVector3(initialPosition.x, initialPosition.y, initialPosition.z));
	startTransform.setRotation(btQuaternion(initialOrientation.x, initialOrientation.y, initialOrientation.z));

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
	// SetActivationState is used to prevent the object properties from being deactivated due to inactivity
	rigidBody->setActivationState(DISABLE_DEACTIVATION);
	if (collide) {
		world->addRigidBody(rigidBody);

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
