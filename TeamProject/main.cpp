#include <memory>

#include <NCLCoreClasses/Window.h>
#include <NCLCoreClasses/GameTimer.h>
#include <CSC8503CoreClasses/Debug.h>

#include "TutorialGame.h"
#include "NavMesh.h"
#include "Config.h"
#include "PushdownState.h"
#include "PushdownMachine.h"
#include "MainMenuScreen.h"
#include "GameScreen.h"
#include "PauseScreen.h"
#include "EndScreenMP.h"
#include "EndScreenSP.h"

#include "Multiplayer/GamePacketHandlers.hpp"

#ifdef CSC_USE_SDL2
#include "SDLJoystick.h"
#include "HybridController.h"
#endif

#ifndef __PROSPERO__
#include "GameTechRenderer.h"
#else
#include "GameTechAGCRenderer.h"
#include "PS5Core/PS5Window.h"

size_t sceUserMainThreadStackSize = 2 * 1024 * 1024;
extern const char sceUserMainThreadName[] = "TeamProjectGameMain";
int sceUserMainThreadPriority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
size_t sceLibcHeapSize = 256 * 1024 * 1024;
#endif // !__PROSPERO__

using namespace NCL;
using namespace NCL::CSC8503;


std::unique_ptr<Window> createWindow(const Config& config) {
#ifndef __PROSPERO__
    WindowInitialisation options = {
        .width = config.get<uint32_t>("windowWidth"),
        .height = config.get<uint32_t>("windowHeight"),
        .fullScreen = config.get<FullScreenState>("fullscreen"),
        .windowTitle = "Team Project",
    };

    std::unique_ptr<Window> window(Window::CreateGameWindow(options));
    if (!window || !window->HasInitialised()) {
        return nullptr;
    }

    return window;
#else
    return std::make_unique<PS5::PS5Window>("TeamProject", 1920, 1080);
#endif // !__PROSPERO__
}

std::unique_ptr<GameTechRendererInterface> createRenderer(Window* window) {
#ifndef __PROSPERO__
    return std::make_unique<GameTechRenderer>(window);
#else
    return std::make_unique<GameTechAGCRenderer>(window);
#endif // !__PROSPERO__
}

Controller* createController(Window* window) {
#ifndef __PROSPERO__
    auto keyboard = new KeyboardMouseController(*window->GetKeyboard(), *window->GetMouse());
    #ifdef CSC_USE_SDL2
        auto joystick = new UnixCode::SDLJoystick(0);
        if (joystick->initOk()) {
            return new HybridController(
                std::unique_ptr<Controller>(keyboard),
                std::unique_ptr<Controller>(joystick)
            );
        }
        delete joystick;
    #endif
    return keyboard;
#else
    return ((PS5::PS5Window*)window)->GetController();
#endif // !__PROSPERO__

}
bool locked = true;

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>

size_t WindowsAllocatedMemory() {
    PROCESS_MEMORY_COUNTERS info;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info))) {
        return info.WorkingSetSize; 
    }
    return 0;
}
#endif

int main(int argc, char** argv) {
    std::function<size_t(void)> GetAllocatedMemory;

#ifdef _WIN32
    GetAllocatedMemory = &WindowsAllocatedMemory;
#endif

    auto config = Config("user-config.jsonc", Assets::DEFAULTCONFIG);
    bool quickStart = config.get<bool>("quickStart");

    auto window = createWindow(config);

    window->ShowOSPointer(false);
    window->LockMouseToWindow(true);

    auto renderer = createRenderer(window.get());
    auto controller = createController(window.get());

    auto game = std::make_unique<TutorialGame>(renderer.get(), controller, config);

    //audioEngine.Init();

    PushdownMachine machine(new MainMenuScreen(controller, game.get(), renderer.get()));

    // Clear delta time to exclude start up time
    window->GetTimer().GetTimeDeltaSeconds();

    bool quit = false;
    
    int fps = 0;
    int frames = 0;
    int megabytes = 0;

    float elapsed = 0;
    float last = 0;

    while (window->UpdateWindow() && !window->GetKeyboard()->KeyPressed(KeyCodes::ESCAPE) && !quit) {
        if (window->GetKeyboard()->KeyPressed(KeyCodes::NUM1)) {
            locked = !locked;
            window->ShowOSPointer(!locked);
            window->LockMouseToWindow(locked);
        }
        float dt = window->GetTimer().GetTimeDeltaSeconds();
        elapsed += dt;
        
        // Update Display Statistics.
        if (elapsed - last >= 0.5) {
            // Framerate
            fps = frames * 2;
            frames = 0;

            // Memory Usage.
            if (GetAllocatedMemory != nullptr) {
                size_t allocatedMemory = GetAllocatedMemory();
                megabytes = (int)allocatedMemory / (1024 * 1024);
            }

            last += 0.5;
        }
        Debug::Print("FPS: " + std::to_string(fps), { 1.875, 0.05 }, { 1, 1, 1, 1 }, 0.5f);
        Debug::Print("RAM: " + std::to_string(megabytes) + "MB", {1.675, 0.05}, {1, 1, 1, 1}, 0.5f);

        controller->Update(dt);
        window->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

        //game->UpdateGame(dt);

        //if (inMenu) {

        //    //Try showing the FMod logo here

        //    if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
        //        selection = std::min(2, selection + 1);
        //    }
        //    if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
        //        selection = std::max(0, selection - 1);
        //    }
        //    if (controller->GetDigital(Controller::DigitalControl::MenuConfirm) || quickStart) {
        //        GameMode mode = static_cast<GameMode>(selection);

        //        if (mode == GameMode::SINGLEPLAYER) {
        //            game->Start();
        //        }
        //        else {
        //            game->StartMultiplayerGame(mode == GameMode::HOST_GAME);
        //        }
        //        inMenu = false;
        //    }

        //    for (int i = 0; i < 3; i++) {
        //        std::string currentItem = menuItems[i];
        //        if (i == selection) currentItem = "> " + currentItem + " <";
        //        else currentItem = "  " + currentItem;
        //        Debug::Print(currentItem, Vector2(0.4f, 0.35f + (0.1f * i)));

        //    }
        //}

        //else {
        //    if (!machine.Update(dt)) {
        //        inMenu = true;
        //    }
        //    //Add a GetState function to PushdownMachine/PushdownState to return the screen it's on, and if it's on PauseScreen
        //    //Pause the game->UpdateGame
        //}

        quit |= !machine.Update(dt);
        game->GetResourceManager()->update(dt);
        renderer->collectFrameObjects(game->GetWorld());
        renderer->drawFrame(dt);
        Debug::UpdateRenderables(dt);
        frames++;
    }
}
