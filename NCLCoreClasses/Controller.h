/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

class Controller	{
public:
	// Between a range:
	// 0..1: Triggers
	// -1..1: Stick
	// Unbounded: Mouse
	// Implementers MAY use a digital input, such as keyboard,
	// in which case the axis will be 0, 1, or -1
	enum class AnalogueControl {
		MoveSidestep,
		MoveUpDown,
		MoveForward,

		LookX,
		LookY,
	};

	// 0 or 1, a keyboard or controller button that is pressed
	// or a trigger moved beyond a threshold
	enum class DigitalControl {
		Fire,
		Jump,
		Sprint,
		Crouch,
	};

	Controller() {

	}
	virtual ~Controller() {

	}

	virtual float	GetAnalogue(AnalogueControl axis) const = 0;
	virtual bool GetDigital(DigitalControl control) const = 0;

	virtual void Update(float dt = 0.0f) {};
};
