#include "JoystickController.h"

#include <iostream>

namespace NCL {
    void JoystickController::Update(float dt) {
        std::swap(buttonStates, previousButtonStates);

        for (int i = 0; i < (int)Button::Max; i++) {
            buttonStates[i] = internalButtonPressed((Button)i);
        }
        for (int i = 0; i < (int)Analogue::Max; i++) {
            analogueStates[i] = internalAnalogueValue((Analogue)i);
        }

        if (buttonPressed(Button::L3, false, true)) {
            sprinting = !sprinting;
        }
        else if (GetAnalogue(AnalogueControl::MoveForward) == 0 && GetAnalogue(AnalogueControl::MoveSidestep) == 0) {
            sprinting = false;
        }
    }

    float JoystickController::GetAnalogue(AnalogueControl control) const
    {
        switch (control)
        {
        case AnalogueControl::MoveSidestep:
            return analogueState(Analogue::LeftStickX);
        case AnalogueControl::MoveUpDown:
            if (buttonPressed(Button::PadUp)) return 1.0f;
            return buttonPressed(Button::PadDown) ? -1.0f : 0.0f;
        case AnalogueControl::MoveForward:
            return -analogueState(Analogue::LeftStickY);
        case AnalogueControl::LookX:
            return (analogueState(Analogue::TrackpadX) + analogueState(Analogue::RightStickX)) * lookSensitivity;
        case AnalogueControl::LookY:
            return (analogueState(Analogue::TrackpadY) + analogueState(Analogue::RightStickY)) * lookSensitivity;
        default:
            assert(false);
        }
    }

    bool JoystickController::GetDigital(DigitalControl button) const
    {
        switch (button)
        {
        case DigitalControl::Fire: return analogueState(Analogue::R2) >= fireThreshold;
        case DigitalControl::Jump: return buttonPressed(Button::A);
        case DigitalControl::Sprint: return sprinting;
        case DigitalControl::Crouch: return buttonPressed(Button::B);

        case DigitalControl::WorldRollLeft: return buttonPressed({ Button::PadLeft, Button::L1 }, false, true);
        case DigitalControl::WorldRollRight: return buttonPressed({ Button::PadRight, Button::R1 }, false, true);
        case DigitalControl::WorldPitchUp: return buttonPressed({ Button::PadUp, Button::PsTriangle }, false, true);
        case DigitalControl::WorldPitchDown: return buttonPressed({ Button::PadDown, Button::PsSquare }, false, true);

        case DigitalControl::DebugBulletOverlay: return buttonPressed(Button::PadUp, true, true);
        case DigitalControl::DebugFreeCam: return buttonPressed(Button::PadRight, true, true);
        case DigitalControl::DebugRespawnRandom: return buttonPressed(Button::PadLeft, true, true);
        case DigitalControl::DebugShowProfiling: return buttonPressed(Button::PadDown, true, true);

        case DigitalControl::MenuDown: return buttonPressed(Button::PadDown, false, true);
        case DigitalControl::MenuUp: return buttonPressed(Button::PadUp, false, true);
        case DigitalControl::MenuConfirm: return buttonPressed(Button::A, false, true);
        case DigitalControl::Pause: case DigitalControl::Unpause:
            return buttonPressed(Button::Start, false, true);
        case DigitalControl::Scoreboard: // We can't use select on PS5 :(, check for it still since it works on other controllers
            return buttonPressed(Button::TrackpadClick, false, true) >= fireThreshold || buttonPressed(Button::Select);
        case DigitalControl::PauseQuit: return buttonPressed(Button::Start, true, true);
        case DigitalControl::ThirdPerson:
        case DigitalControl::DebugReloadWorld:
            return false;
        default:
            assert(false);
        }
    }
}
