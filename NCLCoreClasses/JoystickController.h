#pragma once

#include <array>

#include "Controller.h"

namespace NCL {
    class JoystickController : public Controller {
    public:
        // Joypad buttons, using their XBOX/Steam Deck names & positions
        enum class Button {
            A,
            B,
            X,
            Y,
            PadUp,
            PadDown,
            PadLeft,
            PadRight,
            // Not usable on PS5
            Select,
            Start,
            // L2 and R2 are triggers
            L1,
            L3,
            R1,
            R3,

            // Steam deck back buttons, don't assume these are present
            DeckL4,
            DeckL5,
            DeckR4,
            DeckR5,

            Max,

            // Aliases, to account for different layouts
            PsCross = A,
            PsCircle = B,
            PsSquare = X,
            PsTriangle = Y,
        };

        enum class Analogue {
            LeftStickX,
            LeftStickY,
            RightStickX,
            RightStickY,

            L2,
            R2,

            Max
        };

        virtual ~JoystickController() = default;

        void Update(float dt) override;
		float GetAnalogue(AnalogueControl control) const override;
		bool GetDigital(DigitalControl button) const override;

    protected:
		// If this button is pressed, listen to debug inputs and ignore others
		Button DebugMask = Button::R3;
		float fireThreshold = 0.5f;
		float lookSensitivity = 2.0f;

        // Get the value of a button or analogue input
        // Implementations should return 0 if the button does not exist
        virtual bool internalButtonPressed(Button button) = 0;
        // Value between -1.0f and 1.0f
        virtual float internalAnalogueValue(Analogue analogue) = 0;

        bool buttonPressed(Button button, bool isDebug = false, bool thisFrame = false) const {
            bool debugMask = buttonStates[(int)DebugMask];
            if (debugMask != isDebug) return false;


            bool current = buttonStates[(int)button];
            if (!thisFrame) return current;
            bool previous = previousButtonStates[(int)button];
            return current && !previous;
        }

        bool buttonPressed(std::initializer_list<Button> buttons, bool isDebug = false, bool thisFrame = false) const {
            for (auto button : buttons) {
                if (buttonPressed(button, isDebug, thisFrame)) {
                    return true;
                }
            }
            return false;
        }

        float analogueState(Analogue axis) const {
            return analogueStates[(int)axis];
        }

        std::array<bool, (int)Button::Max> buttonStates;
        std::array<bool, (int)Button::Max> previousButtonStates;
        std::array<float, (int)Analogue::Max> analogueStates;

        bool sprinting = false;
    };
}
