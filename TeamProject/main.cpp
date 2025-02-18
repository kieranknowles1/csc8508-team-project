#include <NCLCoreClasses/Window.h>
#include <NCLCoreClasses/GameTimer.h>

#include "TutorialGame.h"
#include "NavMesh.h"

NCL::Window* createWindow() {
	NCL::WindowInitialisation options = {
		.width = 1280,
		.height = 720,
		.fullScreen = false,
		.windowTitle = "Team Project",
	};

	NCL::Window* window = NCL::Window::CreateGameWindow(options);
	if (!window || !window->HasInitialised()) {
		throw std::runtime_error("Window failed to initialise!");
	}

	return window;
}

int main(int argc, char** argv) {

	auto window = createWindow();

	window->ShowOSPointer(false);
	window->LockMouseToWindow(true);

	auto g = new NCL::CSC8503::TutorialGame();
	// Clear delta time to exclude start up time
	window->GetTimer().GetTimeDeltaSeconds();
	while (window->UpdateWindow() && !NCL::Window::GetKeyboard()->KeyDown(NCL::KeyCodes::ESCAPE)) {
		float dt = window->GetTimer().GetTimeDeltaSeconds();

		window->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);
	}
	delete g;
	// Deleting game destroys the GL context, which should be done before destroying the window
	delete window;

	return 0;
}
