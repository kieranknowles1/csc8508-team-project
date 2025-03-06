#include "Wanderer.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "RenderObject.h"
#include <stdio.h>

using namespace NCL;
using namespace CSC8503;

Wanderer::Wanderer(GameObject* p, NavMesh* mesh, GameObject* g) :
	player(p), navMesh(mesh), gun(g) {
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

	UpdatePlayerDistance();
	stateMachine->Update(dt);

	btTransform trans = GetTransform();
	if (FollowPath(dt, player)) {
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
	SetGunTransform();
	btVector3 vec = GetTransform().getOrigin();
	std::cout << "Player: (" << vec.x() << ", " << vec.y() << ", " << vec.z() << ")" << std::endl;
	vec = gun->GetTransform().getOrigin();
	std::cout << "Gun: (" << vec.x() << ", " << vec.y() << ", " << vec.z() << ")" << std::endl;
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

void Wanderer::PlayerNear() {
	GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	gun->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
}

void Wanderer::PlayerFar() {
	GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	gun->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
}

void Wanderer::SetGunTransform() {
	btTransform gunTransform = gun->GetTransform();
	btTransform wTransform = GetTransform();
	btMatrix3x3 wRotation = wTransform.getBasis();
	btVector3 wPos = wTransform.getOrigin();
	btMatrix3x3 rotationMatrix(wRotation);
	btVector3 forwardDir = rotationMatrix * btVector3(0, 0, -1);
	btVector3 forwardPos = wPos + (forwardDir * btVector3(1.3, -0.7, -1.2));
	gunTransform.setOrigin(forwardPos);
	btRigidBody* gunBody = gun->GetPhysicsObject()->GetRigidBody();
	gunBody->setWorldTransform(gunTransform);
}