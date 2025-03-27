#include "Wanderer.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "RenderObject.h"
#include <stdio.h>
#include "TutorialGame.h"
#include "Health.h"

using namespace NCL;
using namespace CSC8503;

Wanderer::Wanderer(PlayerObject* p, NavMesh* mesh, Side side, GameTechRendererInterface* r, int difficulty) :
	player(p), navMesh(mesh), shootTimer(maxShootTimer), updateplayerPathTimer(maxUpdatePlayerPathTimer), renderer(r), difficulty(difficulty) {

	type = GameObject::Type::AI;
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
		if (playerDist <= senseDistance) {
			btTransform trans = GetTransform();
			btTransform pTrans = player->GetTransform();

			btVector3 wl = trans.getOrigin();
			btVector3 pl = pTrans.getOrigin();

			btVector3 dir = (pl - wl) == 0 ? btVector3(0, 0, 0) : (pl - wl).normalized();
			std::optional<ShotInfo> shot = Shoot::GetInstance()->RayClosest(wl, dir, true, static_cast<GameObject*>(this));
			if (shot.has_value()) {
				if (!(shot.value().hitObj == player || shot.value().hitObj == player->getGun())) return false;
			}
			GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
			/*btVector3 pPos = player->GetTransform().getOrigin();
			pPos.setY(navMesh->GetYFromPoint(pPos.getX(), pPos.getZ()));
			curPath = navMesh->FindPath(curPathPoint, pPos);
			NewPath(curPath);*/
			updateplayerPathTimer = maxUpdatePlayerPathTimer;
			return true;
		}
		else {
			return false;
		}
		}));

	stateMachine->AddTransition(new StateTransition(playerNear, playerFar, [&]()->bool {
		if (playerDist > senseDistance * 2) {
			GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
			curPath = navMesh->FindPath(curPathPoint, navMesh->GetRandomPointInNavMesh());
			if (curPath.size() > 0) NewPath(curPath);
			laser->SetStartPos({ 0, 0, 0 });
			laser->SetEndPos({ 0, 0, 0 });
			isShooting = false;
			attack->Hit(nullptr);
			return true;
		}
		else {
			btTransform trans = GetTransform();
			btTransform pTrans = player->GetTransform();

			btVector3 wl = trans.getOrigin();
			btVector3 pl = pTrans.getOrigin();

			btVector3 dir = (pl - wl) == 0 ? btVector3(0, 0, 0) : (pl - wl).normalized();
			std::optional<ShotInfo> shot = Shoot::GetInstance()->RayClosest(wl, dir, true, static_cast<GameObject*>(this));
			if (shot.has_value()) {
				if (!(shot.value().hitObj == player || shot.value().hitObj == player->getGun())) {
					GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
					curPath = navMesh->FindPath(curPathPoint, navMesh->GetRandomPointInNavMesh());
					if (curPath.size() > 0) NewPath(curPath);
                    laser->SetStartPos({ 0, 0, 0 });
                    laser->SetEndPos({ 0, 0, 0 });

                    isShooting = false;
                    attack->Hit(nullptr);
					return true;
				}
			}
			return false;
		}
		}));


	health = std::make_unique<HealthAttrib>(this);
	health->SetMaxHealth(25.0f * difficulty);
	health->SetCurrentHealth(health->GetMaxHealth());

	attack = std::make_unique<AttackAttrib>();
	attack->SetDamageAmount(10.0f * difficulty);
	attack->SetDamageType(DamageType::CONTINUOUS);
	attack->SetHealthAttrib(health.get());
}

Wanderer::~Wanderer() {
	if (laser && TutorialGame::getInstance()) {
		TutorialGame::getInstance()->delayedRemoveObject(laser);
	}

	laser = nullptr;
	delete stateMachine;
}

void Wanderer::Update(float dt) {
	health->Update(dt);
	attack->Update(dt);

	if (health->GetHealthState() == AliveState::DEAD) {
		DestroyWanderer();
		return;
	}

	UpdatePlayerDistance();
	stateMachine->Update(dt);

}

void Wanderer::DestroyWanderer() {
	TutorialGame::getInstance()->delayedRemoveObject(this);
}

void Wanderer::InitPosAndOffset() {
	btCollisionShape* shape = GetPhysicsObject()->GetRigidBody()->getCollisionShape();
	btCapsuleShape* capsule = static_cast<btCapsuleShape*>(shape);
	float halfHeight = capsule->getHalfHeight();
	btTransform trans = GetTransform();
	btQuaternion rotation = btQuaternion::getIdentity();
	offset = btVector3(0, halfHeight * 2, 0);
	switch(side) {
	case(Side::BOTTOM):
		break;
	case(Side::TOP):
		offset = btVector3(0, -halfHeight * 2, 0);
		rotation = btQuaternion(btVector3(1, 0, 0), SIMD_PI);
		break;
	case(Side::FRONT):
		offset = btVector3(0, 0, -halfHeight * 2);
		rotation = btQuaternion(btVector3(1, 0, 0), -SIMD_PI / 2);
		break;
	case(Side::BACK):
		offset = btVector3(0, 0, halfHeight * 2);
		rotation = btQuaternion(btVector3(1, 0, 0), SIMD_PI / 2);
		break;
	case(Side::LEFT):
		offset = btVector3(halfHeight * 2, 0, 0);
		rotation = btQuaternion(btVector3(0, 0, 1), -SIMD_PI / 2);
		break;
	case(Side::RIGHT):
		offset = btVector3(-halfHeight * 2, 0, 0);
		rotation = btQuaternion(btVector3(0, 0, 1), SIMD_PI / 2);
		break;
	}
	trans.setRotation(rotation);
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

	//wl.setY(0);
	//pl.setY(0);

	playerDist = wl.distance(pl);
}


void Wanderer::PlayerNear(float dt) {

	updateplayerPathTimer -= dt;

	if (isShooting) {

		btTransform trans = GetTransform();
		btVector3 curPos = trans.getOrigin();
		btVector3 pPos = player->GetTransform().getOrigin();
		btVector3 dir = (pPos - curPos) == 0 ? btVector3(0, 0, 0) : (pPos - curPos).normalized();

		std::optional<ShotInfo> info = Shoot::GetInstance()->ShootBulletAI(curPos, dir, trans.getRotation(), dt);

		if (info.has_value()) {
			laser->SetEndPos(info.value().hitPos);
			laser->SetStartPos(curPos);

			if (info->hitObj->getType() == GameObject::Type::Player) {
				attack->Hit(((PlayerObject*)info->hitObj)->GetHealthAttrib());
			}
			else attack->Hit(nullptr);
		}

		shootTimer -= dt;
		if (shootTimer <= 0) {
			isShooting = false;
			attack->Hit(nullptr);
		}
	}
    else {
        btTransform trans = GetTransform();
        btVector3 curPos = trans.getOrigin();
        btVector3 pPos = player->GetTransform().getOrigin();

		if (curPos.distance(pPos) > 50) {
			btVector3 dir = (pPos - curPos) == 0 ? btVector3(0, 0, 0) : (pPos - curPos).normalized(); // Get the direction towards the player

			btVector3 newPos = curPos + (dir * (speed * dt)); // Move towards player

			trans.setOrigin(newPos);
			physicsObject->GetRigidBody()->setWorldTransform(trans);
		}

        laser->SetEndPos({ 0, 0, 0 });
        laser->SetStartPos({ 0, 0, 0 });
        attack->Hit(nullptr);

        shootTimer += dt;
        if (shootTimer >= maxShootTimer) isShooting = true;

    }
}

void Wanderer::PlayerFar(float dt) {

	btTransform trans = GetTransform();
	if (FollowPath(dt)) {
		btVector3 newPos = adjustedPoint + offset;
		trans.setOrigin(newPos);
		btRigidBody* body = physicsObject->GetRigidBody();
		body->setWorldTransform(trans);
		//navMesh->DebugDrawPath(curPath);
	}
	else {
		curPath = navMesh->FindPath(curPathPoint, navMesh->GetRandomPointInNavMesh());
		if (curPath.size() > 0) NewPath(curPath);
	}

}
