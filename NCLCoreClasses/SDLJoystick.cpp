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
            default: return SDL_CONTROLLER_BUTTON_Y; // TODO
        }
    }

    bool SDLJoystick::internalButtonPressed(Button button)
    {
        auto sdl = toSdl(button);
        if (button == Button::A) {
            std::cout << (SDL_GameControllerGetButton(stick, sdl) ? "y" : "n") << std::endl;
        }
        return SDL_GameControllerGetButton(stick, sdl);
    }

    float SDLJoystick::internalAnalogueValue(Analogue analogue)
    {
        return 0.0f;
    }
}
