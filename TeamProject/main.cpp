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


void TestPacketHandlers() {

    Packet::RequestUserIDPacketHandler infoHandler;
    Packet::PacketRegister::Register(&infoHandler);

    std::shared_ptr<Packet::RequestUserIDPacket> testInfoPacket = std::make_shared<Packet::RequestUserIDPacket>(nullptr);

    ENetPacket* userInfoENetPacket = infoHandler.ToENetPacket(testInfoPacket);
    ENetEvent event;
    event.packet = userInfoENetPacket;

    std::shared_ptr<Packet::Packet> infoPacketReturned = infoHandler.Translate(&event);
    std::shared_ptr<Packet::RequestUserIDPacket> infoPacketConverted = std::static_pointer_cast<Packet::RequestUserIDPacket>(infoPacketReturned);
}

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

int main(int argc, char** argv) {
    auto config = Config("user-config.jsonc", Assets::DEFAULTCONFIG);
    bool quickStart = config.get<bool>("quickStart");

    auto window = createWindow(config);

    window->ShowOSPointer(false);
    window->LockMouseToWindow(true);

    auto renderer = createRenderer(window.get());
    auto controller = createController(window.get());

    auto game = std::make_unique<TutorialGame>(renderer.get(), controller, config);

    //PushdownMachine machine(new GameScreen(controller, game.get(), "Singleplayer"));
    //PushdownMachine machine(new MainMenuScreen(controller, game.get()));
    PushdownMachine machine(new EndScreenMP(controller, game.get()));

    // Clear delta time to exclude start up time
    window->GetTimer().GetTimeDeltaSeconds();

    bool quit = false;
    while (window->UpdateWindow() && !window->GetKeyboard()->KeyPressed(KeyCodes::ESCAPE) && !quit) {
        if (window->GetKeyboard()->KeyPressed(KeyCodes::NUM1)) {
            locked = !locked;
            window->ShowOSPointer(!locked);
            window->LockMouseToWindow(locked);
        }
        float dt = window->GetTimer().GetTimeDeltaSeconds();
        controller->Update(dt);
        window->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

        quit |= !machine.Update(dt);
        game->GetResourceManager()->update(dt);
        renderer->collectFrameObjects(game->getWorld());
        renderer->drawFrame(dt);
        Debug::UpdateRenderables(dt);
    }
}
