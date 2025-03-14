#pragma once

#include <NCLCoreClasses/Assets.h>
#include <CSC8503CoreClasses/Debug.h>

#include "PushdownState.h"

namespace NCL::CSC8503 {

class CreditsScreen : public PushdownState {
	// Screen % per second
	const static constexpr float Speed = 10.0f;
	// ~80 chars
	const static constexpr float TextScale = 0.65f;

	float speedup = 0.0f;

	float yPos = 30.0f;
	float totalHeight;
	Controller* controller;
	std::string text;
public:
	CreditsScreen(Controller* controller, std::string file) {
		this->controller = controller;
		bool ok = Assets::ReadTextFile(file, text);
		if (!ok) {
			std::cerr << "Failed to read credits file";
		}

		int lineCount = std::count(text.begin(), text.end(), '\n');
		totalHeight = (lineCount * 5.0f * TextScale);
	}

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (controller->GetDigital(Controller::DigitalControl::Pause)) {
			return PushdownResult::Pop;
		}

		if (-yPos > totalHeight) {
			return PushdownResult::Pop;
		}

		float down = controller->GetAnalogue(Controller::AnalogueControl::MoveForward);
		if (down != 0) {
			speedup -= down * 15.0f * dt;
		}
		else {
			speedup = 0;
		}

		// ~80 chars at 720p
		Debug::Print("Oh hi there. I see you found a feature (TM)", Vector2(0, yPos - 50.0f));
		Debug::Print(text, Vector2(0, yPos), Vector4(1, 1, 1, 1), TextScale);
		yPos -= (Speed + speedup) * dt;

		return PushdownResult::NoChange;
	}
};

}
