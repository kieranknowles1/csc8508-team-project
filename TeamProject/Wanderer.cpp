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
		this->PlayerNear(dt);
	});

	State* playerFar = new State([&](float dt)-> void {
		this->PlayerFar(dt);
	});

	stateMachine->AddState(playerFar);
	stateMachine->AddState(playerNear);

	stateMachine->AddTransition(new StateTransition(playerFar, playerNear, [&]()->bool {
		if (playerDist <= senseDistance) {
			GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
			return true;
		}
		else {
			return false;
		}
		}));

	stateMachine->AddTransition(new StateTransition(playerNear, playerFar, [&]()->bool {
		if (playerDist > senseDistance + 30.0f) {
			/*GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
			curPath = navMesh->FindPath(curPathPoint, navMesh->GetRandomPointInNavMesh());
			NewPath(curPath);*/
			return true;
		}
		else {
			return false;
		}
		}));
}

Wanderer::~Wanderer() {
	delete stateMachine;
}

void Wanderer::Update(float dt) {

	UpdatePlayerDistance();
	stateMachine->Update(dt);

	btTransform trans = GetTransform();
	if (FollowPath(dt)) {
		btVector3 newPos = curPathPoint + offset;
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

void Wanderer::InitPosAndOffset() {
	btCollisionShape* shape = GetPhysicsObject()->GetRigidBody()->getCollisionShape();
	btCapsuleShape* capsule = static_cast<btCapsuleShape*>(shape);
	float halfHeight = capsule->getHalfHeight();
	offset = btVector3(0, halfHeight * 2, 0);
	btTransform trans = GetTransform();
	curPathPoint = trans.getOrigin();
	btVector3 newPos = trans.getOrigin() + offset;
	trans.setOrigin(newPos);
	btRigidBody* body = physicsObject->GetRigidBody();
	body->setWorldTransform(trans);
}

void Wanderer::UpdatePlayerDistance() {
	if (!player) return;

	btTransform trans = GetTransform();
	btTransform pTrans = player->GetTransform();

	btVector3 wl = trans.getOrigin();
	btVector3 pl = pTrans.getOrigin();

	playerDist = wl.distance(pl);
}

void Wanderer::PlayerNear(float dt) {
	
}

void Wanderer::PlayerFar(float dt) {
	/*btTransform trans = GetTransform();
	if (FollowPath(dt)) {
		btVector3 newPos = curPathPoint + offset;
		trans.setOrigin(newPos);
		btRigidBody* body = physicsObject->GetRigidBody();
		body->setWorldTransform(trans);
		navMesh->DebugDrawPath(curPath);
	}
	else {
		curPath = navMesh->FindPath(curPathPoint, navMesh->GetRandomPointInNavMesh());
		NewPath(curPath);
	}*/
}