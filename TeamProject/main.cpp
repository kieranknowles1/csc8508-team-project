#include <memory>

#include <NCLCoreClasses/Window.h>
#include <NCLCoreClasses/GameTimer.h>
#include <CSC8503CoreClasses/Debug.h>

#include "GameTechRenderer.h"
#include "TutorialGame.h"
#include "NavMesh.h"
#include "Config.h"

using namespace NCL;
using namespace NCL::CSC8503;

std::unique_ptr<Window> createWindow(const Config& config) {
	WindowInitialisation options = {
		.width = config.get<uint32_t>("windowWidth"),
		.height = config.get<uint32_t>("windowHeight"),
		.fullScreen = config.get<bool>("fullscreen"),
		.windowTitle = "Team Project",
	};

	std::unique_ptr<Window> window(Window::CreateGameWindow(options));
	if (!window || !window->HasInitialised()) {
		throw std::runtime_error("Window failed to initialise!");
	}

	return window;
}

int main(int argc, char** argv) {
	auto config = Config("user-config.jsonc", "default-config.jsonc");

	auto window = createWindow(config);

	window->ShowOSPointer(false);
	window->LockMouseToWindow(true);

	auto world = std::make_unique<GameWorld>();
	auto renderer = std::make_unique<GameTechRenderer>(world.get());
	auto controller = std::make_unique<KeyboardMouseController>(*window->GetKeyboard(), *window->GetMouse());

	auto game = std::make_unique<TutorialGame>(renderer.get(), world.get(), controller.get());
	// Clear delta time to exclude start up time
	window->GetTimer().GetTimeDeltaSeconds();
	while (window->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
		float dt = window->GetTimer().GetTimeDeltaSeconds();

		window->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		game->UpdateGame(dt);
		renderer->Update(dt);
		renderer->Render();
		Debug::UpdateRenderables(dt);
	}
}
