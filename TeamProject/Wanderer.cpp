#include "Wanderer.h"

using namespace NCL;
using namespace CSC8503;

Wanderer::Wanderer(NavMesh* mesh) :
	navMesh(mesh) {
	navOffset = btVector3(0, 0, 0);
}

void Wanderer::Update(float dt) {

	btTransform trans = GetTransform();
	if (FollowPath(trans.getOrigin() - navOffset, dt)) {
		btVector3 newPos = pathPoint + navOffset;
		trans.setOrigin(newPos);
		btRigidBody* body = physicsObject->GetRigidBody();
		body->setWorldTransform(trans);
		navMesh->DebugDrawPath(curPath);
	}
	else {
		curPath = navMesh->FindPath(GetTransform().getOrigin() - navOffset, navMesh->GetRandomPointInNavMesh());
		NewPath(curPath);
	}
}

void Wanderer::SetOffset() {
	btCollisionShape* shape = GetPhysicsObject()->GetRigidBody()->getCollisionShape();
	btCapsuleShape* capsule = static_cast<btCapsuleShape*>(shape);
	float halfHeight = capsule->getHalfHeight();
	navOffset = btVector3(0, halfHeight * 2, 0);
	btTransform trans = GetTransform();
	btVector3 newPos = trans.getOrigin() + navOffset;
	trans.setOrigin(newPos);
	btRigidBody* body = physicsObject->GetRigidBody();
	body->setWorldTransform(trans);
}