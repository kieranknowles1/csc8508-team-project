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


using namespace NCL;
float	KeyboardMouseController::GetAnalogue(AnalogueControl axis) const {
	switch (axis) {
	case AnalogueControl::MoveForward: return getAxis(KeyCodes::W, KeyCodes::S);
	case AnalogueControl::MoveSidestep: return getAxis(KeyCodes::D, KeyCodes::A);
	case AnalogueControl::MoveUpDown: return getAxis(KeyCodes::SPACE, KeyCodes::SHIFT);
	case AnalogueControl::LookX: return mouse.GetRelativePosition().x;
	case AnalogueControl::LookY: return mouse.GetRelativePosition().y;
	default: assert(false && "Unknown axis");
	}
}

float	KeyboardMouseController::GetButtonAnalogue(uint32_t button) const {
	return GetButton(button);
}

bool	KeyboardMouseController::GetButton(uint32_t button)  const {
	if (button == LeftMouseButton) {
		return mouse.ButtonDown(NCL::MouseButtons::Left);
	}
	if (button == RightMouseButton) {
		return mouse.ButtonDown(NCL::MouseButtons::Right);
	}
	if (button == Jump){
		return keyboard.KeyDown(NCL::KeyCodes::SPACE);
	}
	if (button == Sprint) {
		return keyboard.KeyDown(NCL::KeyCodes::SHIFT);
	}
	if (button == Crouch) {
		return std::max(keyboard.KeyDown(NCL::KeyCodes::CONTROL), (keyboard.KeyDown(NCL::KeyCodes::C)));
	}
	return 0.0f;
}

float KeyboardMouseController::getAxis(KeyCodes::Type positive, KeyCodes::Type negative) const
{
	if (keyboard.KeyDown(positive)) return 1.0f;

	return keyboard.KeyDown(negative) ? -1.0f : 0.0f;
}
