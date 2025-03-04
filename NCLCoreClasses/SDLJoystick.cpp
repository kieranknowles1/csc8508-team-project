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
    }

    SDLJoystick::~SDLJoystick()
    {
        if (stick != nullptr) {
            SDL_GameControllerClose(stick);
        }
    }

    bool SDLJoystick::internalButtonPressed(Button button)
    {
        return false;
    }

    float SDLJoystick::internalAnalogueValue(Analogue analogue)
    {
        return 0.0f;
    }
}
