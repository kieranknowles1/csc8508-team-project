#include "Turret.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

Turret::Turret(Quaternion q) {
	initialRotation = q;
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
		std::cout << "To rotate right" << std::endl;
		}));

	stateMachine->AddTransition(new StateTransition(rotateRight, rotateLeft, [&]()->bool {
		return rotateTime <= 0.0f;
		std::cout << "To rotate left" << std::endl;
		}));
}

Turret::~Turret() {
	delete stateMachine;
}

void Turret::Update(float dt) {
	stateMachine->Update(dt);
}

void Turret::RotateLeft(float dt) {
	rotateTime += rotateSpeed * dt;
	rotateTime = std::clamp(rotateTime, 0.0f, 1.0f);
	std::cout << "left " << rotateTime << std::endl;
	transform.SetOrientation(
		Quaternion::Slerp(yNegative, yPositive, rotateTime)
	);
}

void Turret::RotateRight(float dt) {
	rotateTime -= rotateSpeed * dt;
	rotateTime = std::clamp(rotateTime, 0.0f, 1.0f);
	std::cout << "right " << rotateTime << std::endl;
	transform.SetOrientation(
		Quaternion::Slerp(yNegative, yPositive, rotateTime)

	);

}