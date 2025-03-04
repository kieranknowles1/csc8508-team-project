#pragma once

namespace NCL {
    class JoystickController {
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
            Select,
            Start,
            L1,
            L2,
            L3,
            R1,
            R2,
            R3,

            // Steam deck back buttons, don't assume these are present
            DeckL4,
            DeckL5,
            DeckR4,
            DeckR5
        };

        enum class Analogue {
            LeftStickX,
            LeftStickY,
            RightStickX,
            RightStickY,
        };

    protected:
        // Get the value of a button or analogue input
        // Implementations should return 0 if the button does not exist
        virtual bool internalButtonPressed(Button button) = 0;
        virtual float internalAnalogueValue(Analogue analogue) = 0;
    };
}
