#include "SDLMouse.h"

#include <cassert>

namespace NCL::UnixCode {
    MouseButtons::Type toWindowsButton(Uint8 sdlButton)
    {
        switch (sdlButton)
        {
            case SDL_BUTTON_LEFT:
                return MouseButtons::Left;
            case SDL_BUTTON_RIGHT:
                return MouseButtons::Right;
            case SDL_BUTTON_MIDDLE:
                return MouseButtons::Middle;
            case SDL_BUTTON_X1:
                return MouseButtons::Four;
            case SDL_BUTTON_X2:
                return MouseButtons::Five;
            default:
                assert(false);
        }
    }

    void SDLMouse::handleEvent(const SDL_MouseButtonEvent &event)
    {
        bool down = event.state == SDL_PRESSED;
        auto winButton = toWindowsButton(event.button);

        buttons[winButton] = down;
    }
    void SDLMouse::handleEvent(const SDL_MouseMotionEvent &event)
    {
        relativePosition.x += event.xrel * sensitivity;
        relativePosition.y += event.yrel * sensitivity;
        absolutePosition.x = event.x;
        absolutePosition.y = event.y;
    }
}
