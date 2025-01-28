#pragma once

// SDL2 defines its own main function, so avoid including the main header
#include <SDL2/SDL_video.h>

#include "Window.h"

#include "SDLKeyboard.h"
#include "SDLMouse.h"

namespace NCL::UnixCode {
    class SDLWindow : public Window {
public:
    SDLWindow(const WindowInitialisation& init);

    void LockMouseToWindow(bool lock) override;
    void ShowOSPointer(bool show) override;

    void SetWindowPosition(int x, int y) override;
    void SetFullScreen(bool state) override;
    void SetConsolePosition(int x, int y) override;
    void ShowConsole(bool state) override;

    SDL_Window* getSdlWindow() const { return sdlWindow; }
protected:
    SDL_Window* sdlWindow;

    bool InternalUpdate() override;

    void UpdateTitle() override;

    SDLKeyboard* sdlKeyboard;
    SDLMouse* sdlMouse;
    };
}
