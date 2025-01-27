#include "SDLKeyboard.h"

namespace NCL::UnixCode {
    void SDLKeyboard::handleEvent(const SDL_KeyboardEvent &event)
    {
        bool down = event.state == SDL_PRESSED;
        SDL_Keycode key = event.keysym.sym;
        KeyCodes::Type keyCode = convertKey(key);
        keyStates[keyCode] = down;
    }

    KeyCodes::Type SDLKeyboard::convertKey(SDL_Keycode key) {
        // SDL2 uses lowercase letters for keycodes
        // Windows uses uppercase
        if (key >= SDLK_a && key <= SDLK_z) {
            return (KeyCodes::Type)(key - ('a' - 'A'));
        }

        // F1-F24 range from 0x70 to 0x87
        if (key >= SDLK_F1 && key <= SDLK_F24) {
            int offset = key - SDLK_F1;
            return (KeyCodes::Type)(KeyCodes::F1 + offset);
        }

        switch (key) {
            case SDLK_LSHIFT:
                return KeyCodes::SHIFT;
            case SDLK_RSHIFT:
                return KeyCodes::SHIFT;
        }

        if (key >= KeyCodes::MAXVALUE) {
            std::cerr << "Unknown key code: " << key << std::endl;
            return KeyCodes::MAXVALUE;
        }

        // Assume the keycodes are the same, plenty of cases we're wrong but it's a start
        return (KeyCodes::Type)key;
    }
}
