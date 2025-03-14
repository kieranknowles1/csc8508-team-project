#pragma once

#include <NCLCoreClasses/Assets.h>
#include <CSC8503CoreClasses/Debug.h>

#include "PushdownState.h"

namespace NCL::CSC8503 {

class CreditsScreen : public PushdownState {
	const static constexpr float Speed = 10.0f;

	float yPos = 30.0f;
	Controller* controller;
	std::string text;
public:
	CreditsScreen(Controller* controller, std::string file) {
		this->controller = controller;
		bool ok = Assets::ReadTextFile(file, text);
		if (!ok) {
			std::cerr << "Failed to read credits file";
		}
	}

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (controller->GetDigital(Controller::DigitalControl::Pause)) {
			return PushdownResult::Pop;
		}

		// ~80 chars at 720p
		Debug::Print(text, Vector2(0, yPos), Vector4(1, 1, 1, 1), 0.65f);
		yPos -= Speed * dt;

		return PushdownResult::NoChange;
	}
};

}
