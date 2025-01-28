#pragma once

#include <SDL2/SDL_events.h>

#include "Mouse.h"

namespace NCL::UnixCode {
    class SDLMouse : public Mouse {
    public:
        void handleEvent(const SDL_MouseButtonEvent& event);
        void handleEvent(const SDL_MouseMotionEvent& event);
    };
}
