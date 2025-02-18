/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "KeyboardMouseController.h"

#include <assert.h>

#include "Mouse.h"
#include "Keyboard.h"


namespace NCL {
float KeyboardMouseController::GetAnalogue(AnalogueControl axis) const {
	switch (axis) {
	case AnalogueControl::MoveForward: return getAxis(KeyCodes::W, KeyCodes::S);
	case AnalogueControl::MoveSidestep: return getAxis(KeyCodes::D, KeyCodes::A);
	case AnalogueControl::MoveUpDown: return getAxis(KeyCodes::SPACE, KeyCodes::SHIFT);
	case AnalogueControl::LookX: return mouse.GetRelativePosition().x;
	case AnalogueControl::LookY: return mouse.GetRelativePosition().y;
	default: assert(false && "Unknown axis");
	}
}

bool KeyboardMouseController::GetDigital(DigitalControl button)  const {
	switch (button) {
	case DigitalControl::Fire: return mouse.ButtonDown(MouseButtons::Left);
	case DigitalControl::Jump: return keyboard.KeyDown(KeyCodes::SPACE);
	case DigitalControl::Sprint: return keyboard.KeyDown(KeyCodes::SHIFT);
	case DigitalControl::Crouch: return keyboard.KeyDown(KeyCodes::CONTROL) || keyboard.KeyDown(KeyCodes::C);
	case Controller::DigitalControl::DebugBulletOverlay:
		return keyboard.KeyPressed(KeyCodes::F3);
	case Controller::DigitalControl::DebugFreeCam:
		return keyboard.KeyPressed(KeyCodes::F);
	case Controller::DigitalControl::DebugReloadWorld:
		return keyboard.KeyPressed(KeyCodes::F1);
	case Controller::DigitalControl::DebugShowProfiling:
		return keyboard.KeyPressed(KeyCodes::F4);
	case Controller::DigitalControl::RotateWorld:
		return keyboard.KeyPressed(KeyCodes::V);
	case Controller::DigitalControl::ThirdPerson:
		return keyboard.KeyPressed(KeyCodes::G);
	default: assert(false && "Unknown axis");
	}
}

float KeyboardMouseController::getAxis(KeyCodes::Type positive, KeyCodes::Type negative) const
{
	if (keyboard.KeyDown(positive)) return 1.0f;

	return keyboard.KeyDown(negative) ? -1.0f : 0.0f;
}
}