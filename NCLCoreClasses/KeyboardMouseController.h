/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Controller.h"

#include "Keyboard.h"

namespace NCL {
	class Keyboard;
	class Mouse;

	class KeyboardMouseController : public Controller {
	public:
		KeyboardMouseController(const Keyboard& k, const Mouse& m) : keyboard(k), mouse(m) {
		}

		virtual ~KeyboardMouseController() {
		}

		float	GetAnalogue(AnalogueControl axis) const override;
		bool GetDigital(DigitalControl control) const override;

	protected:
		const Keyboard&		keyboard;
		const Mouse&		mouse;

		float getAxis(KeyCodes::Type positive, KeyCodes::Type negative) const;
	};
}
