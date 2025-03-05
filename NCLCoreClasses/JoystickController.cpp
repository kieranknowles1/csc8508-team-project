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
            return analogueState(Analogue::RightStickX);
        case AnalogueControl::LookY:
            return analogueState(Analogue::RightStickY);
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
        case DigitalControl::Sprint: return buttonPressed(Button::L3);
        case DigitalControl::Crouch: return buttonPressed(Button::B);
        case DigitalControl::ThirdPerson: return buttonPressed(Button::R1, false, true);

        case DigitalControl::WorldRollLeft: return buttonPressed(Button::PadLeft, false, true);
        case DigitalControl::WorldRollRight: return buttonPressed(Button::PadRight, false, true);
        case DigitalControl::WorldPitchUp: return buttonPressed(Button::PadUp, false, true);
        case DigitalControl::WorldPitchDown: return buttonPressed(Button::PadDown, false, true);

        case DigitalControl::DebugBulletOverlay: return buttonPressed(Button::PadUp, true, true);
        case DigitalControl::DebugFreeCam: return buttonPressed(Button::PadRight, true, true);
        case DigitalControl::DebugReloadWorld: return buttonPressed(Button::PadLeft, true, true);
        case DigitalControl::DebugShowProfiling: return buttonPressed(Button::PadDown, true, true);

        case DigitalControl::MenuDown: return buttonPressed(Button::PadDown, false, true);
        case DigitalControl::MenuUp: return buttonPressed(Button::PadUp, false, true);
        case DigitalControl::MenuConfirm: return buttonPressed(Button::A, false, true);
        case DigitalControl::Pause: case DigitalControl::Unpause:
            return buttonPressed(Button::Select, false, true);
        case DigitalControl::PauseQuit: return buttonPressed(Button::Select, true, true);

        default:
            assert(false);
        }
    }
}
