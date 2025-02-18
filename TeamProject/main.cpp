#include <memory>

#include <NCLCoreClasses/Window.h>
#include <NCLCoreClasses/GameTimer.h>
#include <CSC8503CoreClasses/Debug.h>

#include "GameTechRenderer.h"
#include "TutorialGame.h"
#include "NavMesh.h"

std::unique_ptr<NCL::Window> createWindow() {
	NCL::WindowInitialisation options = {
		.width = 1280,
		.height = 720,
		.fullScreen = false,
		.windowTitle = "Team Project",
	};

	std::unique_ptr<NCL::Window> window(NCL::Window::CreateGameWindow(options));
	if (!window || !window->HasInitialised()) {
		throw std::runtime_error("Window failed to initialise!");
	}

	return window;
}

int main(int argc, char** argv) {

	auto window = createWindow();

	window->ShowOSPointer(false);
	window->LockMouseToWindow(true);

	auto world = std::make_unique<GameWorld>();
	auto renderer = std::make_unique<GameTechRenderer>(world.get());
	auto controller = std::make_unique<KeyboardMouseController>(*window->GetKeyboard(), *window->GetMouse());

	auto game = std::make_unique<NCL::CSC8503::TutorialGame>(renderer.get(), world.get(), controller.get());
	// Clear delta time to exclude start up time
	window->GetTimer().GetTimeDeltaSeconds();
	while (window->UpdateWindow() && !NCL::Window::GetKeyboard()->KeyDown(NCL::KeyCodes::ESCAPE)) {
		float dt = window->GetTimer().GetTimeDeltaSeconds();

		window->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		game->UpdateGame(dt);
		renderer->Update(dt);
		renderer->Render();
		Debug::UpdateRenderables(dt);
	}
}
