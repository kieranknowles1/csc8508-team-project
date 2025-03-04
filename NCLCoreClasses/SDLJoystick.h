#pragma once

#include "JoystickController.h"

namespace NCL::UnixCode {
    class SDLJoystick : public JoystickController {
    public:

    protected:
        bool internalButtonPressed(Button button) override;
        float internalAnalogueValue(Analogue analogue) override;
    };
}
