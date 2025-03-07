#pragma once

#include <SDL2/SDL_gamecontroller.h>

#include "JoystickController.h"

namespace NCL::UnixCode {
    class SDLJoystick : public JoystickController {
    public:
        SDLJoystick(int id);
        ~SDLJoystick() override;

        bool initOk() { return ok; }

    protected:
        // TODO: Don't hardcode this
        int deadzone = 4096;

        bool internalButtonPressed(Button button) override;
        float internalAnalogueValue(Analogue analogue) override;

        SDL_GameController* stick = nullptr;
        bool ok = false;
    };
}
