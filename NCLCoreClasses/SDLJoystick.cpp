#include "SDLJoystick.h"

#include "SDL2/SDL.h"

namespace NCL::UnixCode {
    SDLJoystick::SDLJoystick(int id)
    {
        SDL_Init(SDL_INIT_JOYSTICK);
        int count = SDL_NumJoysticks();
        if (count <= id) {
            std::cerr << "Invalid joystick ID. Requested stick " << id << " but only " << count << " are available" << std::endl;
            return;
        }

        stick = SDL_GameControllerOpen(id);

        const char* name = SDL_GameControllerName(stick);
        std::cout << "Using joystick " << name << std::endl;

        ok = true;
    }

    SDLJoystick::~SDLJoystick()
    {
        if (stick != nullptr) {
            SDL_GameControllerClose(stick);
        }
    }

    SDL_GameControllerButton toSdl(JoystickController::Button button) {
        using enum JoystickController::Button;
        switch (button) {
            case A: return SDL_CONTROLLER_BUTTON_A;
            case B: return SDL_CONTROLLER_BUTTON_B;
            case X: return SDL_CONTROLLER_BUTTON_X;
            case Y: return SDL_CONTROLLER_BUTTON_Y;
            case Select: return SDL_CONTROLLER_BUTTON_BACK;
            case Start: return SDL_CONTROLLER_BUTTON_START;
            case L3: return SDL_CONTROLLER_BUTTON_LEFTSTICK;
            case R3: return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
            case L1: return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
            case R1: return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
            case PadUp: return SDL_CONTROLLER_BUTTON_DPAD_UP;
            case PadDown: return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
            case PadLeft: return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
            case PadRight: return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
            case DeckR4: return SDL_CONTROLLER_BUTTON_PADDLE1;
            case DeckR5: return SDL_CONTROLLER_BUTTON_PADDLE2;
            case DeckL4: return SDL_CONTROLLER_BUTTON_PADDLE3;
            case DeckL5: return SDL_CONTROLLER_BUTTON_PADDLE4;
            default: assert(false);
        }
    }

    bool SDLJoystick::internalButtonPressed(Button button)
    {
        auto sdl = toSdl(button);
        return SDL_GameControllerGetButton(stick, sdl);
    }

    SDL_GameControllerAxis toSdlAxis(JoystickController::Analogue axis) {
        using enum JoystickController::Analogue;
        switch (axis) {
            case LeftStickX: return SDL_CONTROLLER_AXIS_LEFTX;
            case LeftStickY: return SDL_CONTROLLER_AXIS_LEFTY;
            case RightStickX: return SDL_CONTROLLER_AXIS_RIGHTX;
            case RightStickY: return SDL_CONTROLLER_AXIS_RIGHTY;
            case L2: return SDL_CONTROLLER_AXIS_TRIGGERLEFT;
            case R2: return SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
            default: assert(false);
        }
    }

    float SDLJoystick::internalAnalogueValue(Analogue analogue)
    {
        auto sdl = toSdlAxis(analogue);
        int raw = SDL_GameControllerGetAxis(stick, sdl);
        if (std::abs(raw) < deadzone) return 0;
        return float(raw) / 32868;
    }
}
