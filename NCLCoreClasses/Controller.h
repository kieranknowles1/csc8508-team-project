/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

class Controller	{
public:
	enum class DigitalControl {

	};

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

	Controller() {

	}
	virtual ~Controller() {

	}

	virtual float	GetAnalogue(AnalogueControl axis) const = 0;

	virtual float	GetButtonAnalogue(uint32_t button) const = 0;
	virtual bool	GetButton(uint32_t button) const = 0;

	virtual float	GetNamedButtonAnalogue(const std::string& button) const;
	virtual bool	GetNamedButton(const std::string& button) const;

	void MapButton(uint32_t axis, const std::string& name);
	void MapButtonAnalogue(uint32_t axis, const std::string& name);

	virtual void Update(float dt = 0.0f) {};

protected:

	std::map<std::string, uint32_t> buttonMappings;
	std::map<std::string, uint32_t> analogueMappings;
};
