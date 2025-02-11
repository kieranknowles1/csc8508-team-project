#include "Turret.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include <numbers>

using namespace NCL;
using namespace CSC8503;

Turret::Turret(GameObject* p, Quaternion q) 
	: player(p), initialRotation(q) {
	stateMachine = new StateMachine();
	rotateTime = 0.5;
	rotateSpeed = 0.5;

	State* rotateLeft = new State([&](float dt)-> void{
		this->RotateLeft(dt);
	});
	State* rotateRight = new State([&](float dt)->void {
		this->RotateRight(dt);
	});

	stateMachine->AddState(rotateLeft);
	stateMachine->AddState(rotateRight);

	yPositive = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), 45.0f);
	yNegative = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), -45.0f);

	stateMachine->AddTransition(new StateTransition(rotateLeft, rotateRight, [&]()->bool {
		return rotateTime >= 1.0f;
		}));

	stateMachine->AddTransition(new StateTransition(rotateRight, rotateLeft, [&]()->bool {
		return rotateTime <= 0.0f;
		}));
}

Turret::~Turret() {
	delete stateMachine;
}

void Turret::Update(float dt) {
	trans = GetTransform();

	stateMachine->Update(dt);

	if (!player) { std::cout << "Player is nullptr to turret" << std::endl; return; }

	btVector3 turretForward = trans.getBasis() * btVector3(0, 0, 1);

	btVector3 toPlayer = player->GetTransform().getOrigin() - trans.getOrigin();
	toPlayer.normalize();

	float dotProduct = turretForward.dot(toPlayer);

	float angleRadians = acos(dotProduct);
	float angleDegrees = angleRadians * (180.0f / std::numbers::pi);

	if (angleDegrees <= 10.0f) {
		std::cout << "Player can be shot by turret " << angleDegrees << std::endl;
	}

}

// TODO: This could be one function
void Turret::RotateLeft(float dt) {
	rotateTime += rotateSpeed * dt;
	rotateTime = std::clamp(rotateTime, 0.0f, 1.0f);

	btQuaternion negative(yNegative.x, yNegative.y, yNegative.z, yNegative.w);
	btQuaternion positive(yPositive.x, yPositive.y, yPositive.z, yPositive.w);
	
	trans.setRotation(negative.slerp(positive, rotateTime));
	GetPhysicsObject()->GetRigidBody()->setWorldTransform(trans);
}

void Turret::RotateRight(float dt) {
	rotateTime -= rotateSpeed * dt;
	rotateTime = std::clamp(rotateTime, 0.0f, 1.0f);

	btQuaternion negative(yNegative.x, yNegative.y, yNegative.z, yNegative.w);
	btQuaternion positive(yPositive.x, yPositive.y, yPositive.z, yPositive.w);

	trans.setRotation(negative.slerp(positive, rotateTime));
	GetPhysicsObject()->GetRigidBody()->setWorldTransform(trans);
}