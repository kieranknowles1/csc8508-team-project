#include <stdexcept>

#include <NCLCoreClasses/Window.h>
#include <NCLCoreClasses/GameTimer.h>

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
	

	// Clear delta time to exclude start up time
	window->GetTimer().GetTimeDeltaSeconds();
	// TODO: Way to exit from ingame, controller support
	while (window->UpdateWindow() && !NCL::Window::GetKeyboard()->KeyDown(NCL::KeyCodes::ESCAPE)) {
		// TODO: Update loop
	}
}
