#include "Wanderer.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "RenderObject.h"
#include <stdio.h>

using namespace NCL;
using namespace CSC8503;

Wanderer::Wanderer(GameObject* p, NavMesh* mesh) :
	player(p), navMesh(mesh) {
	stateMachine = new StateMachine();

	State* playerNear = new State([&](float dt)-> void {
		this->PlayerNear();
	});

	State* playerFar = new State([&](float dt)-> void {
		this->PlayerFar();
	});

	stateMachine->AddState(playerFar);
	stateMachine->AddState(playerNear);

	stateMachine->AddTransition(new StateTransition(playerFar, playerNear, [&]()->bool {
		return playerDist <= senseDistance;
		}));

	stateMachine->AddTransition(new StateTransition(playerNear, playerFar, [&]()->bool {
		return playerDist > senseDistance;
		}));
}

Wanderer::~Wanderer() {
	delete stateMachine;
}

void Wanderer::Update(float dt) {

	canSeePlayer();
	stateMachine->Update(dt);

	btTransform trans = GetTransform();
	if (FollowPath(dt)) {
		btVector3 newPos = newPathPoint;
		trans.setOrigin(newPos);
		btRigidBody* body = physicsObject->GetRigidBody();
		body->setWorldTransform(trans);
		navMesh->DebugDrawPath(curPath);
	}
	else {
		curPath = navMesh->FindPath(curPathPoint, navMesh->GetRandomPointInNavMesh());
		NewPath(curPath);
	}
}

/*void Wanderer::SetOffset() {
	btCollisionShape* shape = GetPhysicsObject()->GetRigidBody()->getCollisionShape();
	btCapsuleShape* capsule = static_cast<btCapsuleShape*>(shape);
	float halfHeight = capsule->getHalfHeight();
	navOffset = btVector3(0, halfHeight * 2, 0);
	btTransform trans = GetTransform();
	btVector3 newPos = trans.getOrigin() + navOffset;
	trans.setOrigin(newPos);
	btRigidBody* body = physicsObject->GetRigidBody();
	body->setWorldTransform(trans);
}*/

void Wanderer::canSeePlayer() {
	if (!player) return;

	btTransform trans = GetTransform();
	btTransform pTrans = player->GetTransform();

	btVector3 wl = trans.getOrigin();
	btVector3 pl = pTrans.getOrigin();

	playerDist = wl.distance(pl);
}

void Wanderer::PlayerNear() {
	GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
}

void Wanderer::PlayerFar() {
	GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
}