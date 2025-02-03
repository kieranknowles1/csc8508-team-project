#include "PhysicsObject.h"
#include "PhysicsSystem.h"
#include "Transform.h"

#include <bullet/btBulletDynamicsCommon.h>

using namespace NCL;
using namespace CSC8503;

PhysicsObject::PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume) 
	: transform(parentTransform), volume(parentVolume), rigidBody(nullptr), motionState(nullptr), collisionShape(nullptr) {
	
	// Other physics properties would be implemented here
	inverseMass = 1.0f;
	elasticity	= 0.8f;
	friction	= 0.8f;
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
void PhysicsObject::InitBulletPhysics(btDynamicsWorld* world, btCollisionShape* shape, float mass) {
	collisionShape = shape;
	btTransform startTransform;
	startTransform.setIdentity();

	// Setting the starting position of the object using the NCL framework's transform
	startTransform.setOrigin(btVector3(transform->GetPosition().x, transform->GetPosition().y, transform->GetPosition().z));

	// MotionState has been used to retrieve and apply Bullet's physics transformations to the NCL object
	motionState = new btDefaultMotionState(startTransform);

	btVector3 localInertia(0, 0, 0);
	if (mass > 0.0f) {
		shape->calculateLocalInertia(mass, localInertia);
	}

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
	rigidBody = new btRigidBody(rbInfo);

	// Setting the object's properties
	rigidBody->setMassProps(mass, localInertia);

	// SetActivationState is used to prevent the object properties from being deactivated due to inactivity
	rigidBody->setActivationState(DISABLE_DEACTIVATION);

	/*rigidBody->setRestitution(elasticity);
	rigidBody->setFriction(friction);
	rigidBody->setDamping(0.99f, 0.99f);*/
	// Log mass and inverse mass of the object
	world->addRigidBody(rigidBody);
}

// Updating the transform from Bullet's physics
void PhysicsObject::UpdateFromBullet() {
	if (!rigidBody || !motionState) return;

	btTransform trans;
	rigidBody->getMotionState()->getWorldTransform(trans);

	// Converting Bullet's matrix to NCL's Transform and updating it
	transform->SetPosition(Vector3(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z()));
	transform->SetOrientation(Quaternion(trans.getRotation().w(), trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z()));
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

/* Bullet Physics Implementaton ends here*/

/* Physics properties implemented using NCL's framework has been commented out, I might remove it as well */

/*
void PhysicsObject::ApplyAngularImpulse(const Vector3& force) {
	angularVelocity += inverseInteriaTensor * force;
}

void PhysicsObject::ApplyLinearImpulse(const Vector3& force) {
	linearVelocity += force * inverseMass;
}

void PhysicsObject::AddForce(const Vector3& addedForce) {
	force += addedForce;
}

void PhysicsObject::AddForceAtPosition(const Vector3& addedForce, const Vector3& position) {
	Vector3 localPos = position - transform->GetPosition();

	force  += addedForce;
	torque += Vector::Cross(localPos, addedForce);
}

void PhysicsObject::AddTorque(const Vector3& addedTorque) {
	torque += addedTorque;
}

void PhysicsObject::ClearForces() {
	force				= Vector3();
	torque				= Vector3();
}
*/

void PhysicsObject::InitCubeInertia() {
	Vector3 dimensions	= transform->GetScale();

	Vector3 fullWidth = dimensions * 2.0f;

	Vector3 dimsSqr		= fullWidth * fullWidth;

	inverseInertia.x = (12.0f * inverseMass) / (dimsSqr.y + dimsSqr.z);
	inverseInertia.y = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.z);
	inverseInertia.z = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.y);
}

void PhysicsObject::InitSphereInertia() {

	float radius	= Vector::GetMaxElement(transform->GetScale());
	float i			= 2.5f * inverseMass / (radius*radius);

	inverseInertia	= Vector3(i, i, i);
}

void PhysicsObject::UpdateInertiaTensor() {
	Quaternion q = transform->GetOrientation();

	Matrix3 invOrientation = Quaternion::RotationMatrix<Matrix3>(q.Conjugate());
	Matrix3 orientation = Quaternion::RotationMatrix<Matrix3>(q);

	inverseInteriaTensor = orientation * Matrix::Scale3x3(inverseInertia) *invOrientation;
}