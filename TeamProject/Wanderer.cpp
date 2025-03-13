#include "Wanderer.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "RenderObject.h"
#include <stdio.h>

using namespace NCL;
using namespace CSC8503;

Wanderer::Wanderer(GameObject* p, NavMesh* mesh, char side) :
	player(p), navMesh(mesh), shootTimer(maxShootTimer), updateplayerPathTimer(maxUpdatePlayerPathTimer){
	this->side = side;
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
		return false;
		if (playerDist <= senseDistance) {
			GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
			shootTimer = maxShootTimer / 2;
			btVector3 pPos = player->GetTransform().getOrigin();
			pPos.setY(navMesh->GetYFromPoint(pPos.getX(), pPos.getZ()));
			curPath = navMesh->FindPath(curPathPoint, pPos);
			NewPath(curPath);
			updateplayerPathTimer = maxUpdatePlayerPathTimer;
			return true;
		}
		else {
			return false;
		}
		}));

	stateMachine->AddTransition(new StateTransition(playerNear, playerFar, [&]()->bool {
		if (playerDist > senseDistance + 30.0f) {
			GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
			curPath = navMesh->FindPath(curPathPoint, navMesh->GetRandomPointInNavMesh());
			NewPath(curPath);
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

}

void Wanderer::InitPosAndOffset() {
	btCollisionShape* shape = GetPhysicsObject()->GetRigidBody()->getCollisionShape();
	btCapsuleShape* capsule = static_cast<btCapsuleShape*>(shape);
	float halfHeight = capsule->getHalfHeight();
	btTransform trans = GetTransform();
	btQuaternion rotation;
	switch(side) {
	case('b'):
		offset = btVector3(0, halfHeight * 2, 0);
		break;
	case('t'):
		offset = btVector3(0, -halfHeight * 2, 0);
		break;
	case('f'):
		offset = btVector3(0, 0, 0);
		rotation = btQuaternion(btVector3(1, 0, 0), SIMD_PI / 2);
		trans.setRotation(rotation);
		break;
	case('k'):
		offset = btVector3(0, halfHeight * 4, 0);
		rotation = btQuaternion(btVector3(1, 0, 0), -SIMD_PI / 2);
		trans.setRotation(rotation);
		break;
	case('l'):
		offset = btVector3(halfHeight * 2, 0, 0);
		rotation = btQuaternion(btVector3(0, 0, 1), -SIMD_PI / 2);
		trans.setRotation(rotation);
		break;
	case('r'):
		offset = btVector3(-halfHeight * 2, 0, 0);
		rotation = btQuaternion(btVector3(0, 0, 1), -SIMD_PI / 2);
		trans.setRotation(rotation);
		break;
	}
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

	wl.setY(0);
	pl.setY(0);

	playerDist = wl.distance(pl);
}


void Wanderer::PlayerNear(float dt) {
	
	shootTimer -= dt;
	updateplayerPathTimer -= dt;

	if (shootTimer <= 0) {

		btVector3 curPos = GetTransform().getOrigin();
		btVector3 pPos = player->GetTransform().getOrigin();
		btVector3 dir = (pPos - curPos) == 0 ? btVector3(0, 0, 0) : (pPos - curPos).normalized();

		Shoot::GetInstance()->ShootBulletPlayer(curPos, dir, GetTransform().getRotation());

		shootTimer = maxShootTimer;
	}
	else {
		btTransform trans = GetTransform();
		btVector3 curPos = trans.getOrigin();
		btVector3 pPos = player->GetTransform().getOrigin();
		if (curPos.distance(pPos) < 30) return;
		pPos.setY(navMesh->GetYFromPoint(pPos.getX(), pPos.getZ()));
		btVector3 dir = (pPos - curPos) == 0 ? btVector3(0, 0, 0) : (pPos - curPos).normalized();
		btVector3 newPos = curPos + dir * speed;
		newPos.setY(newPos.getY() - GroundAdjust(newPos));
		newPos = newPos + offset;
		trans.setOrigin(newPos);
		btRigidBody* body = physicsObject->GetRigidBody();
		body->setWorldTransform(trans);

		/*if (updateplayerPathTimer > 0) {
			if (FollowPath(dt)) {
				btVector3 newPos = yAdjustedPoint + offset;
				btTransform trans = GetTransform();
				trans.setOrigin(newPos);
				btRigidBody* body = physicsObject->GetRigidBody();
				body->setWorldTransform(trans);
				navMesh->DebugDrawPath(curPath);
			}
		}
		else {
			btVector3 pPos = player->GetTransform().getOrigin();
			pPos.setY(navMesh->GetYFromPoint(pPos.getX(), pPos.getZ()));
			curPath = navMesh->FindPath(curPathPoint, pPos);
			NewPath(curPath);
			updateplayerPathTimer = maxUpdatePlayerPathTimer;
		}*/
	}

}

void Wanderer::PlayerFar(float dt) {

	btTransform trans = GetTransform();
	if (FollowPath(dt)) {
		btVector3 newPos = yAdjustedPoint + offset;
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