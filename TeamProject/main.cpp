#include <stdexcept>
#include <iostream>

#include <NCLCoreClasses/Window.h>
#include <NCLCoreClasses/GameTimer.h>
#include <OpenGLRendering/OGLRenderer.h>

#include <OpenGLRendering/glad/gl.h>

#include "Renderer.h"

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
	auto renderer = new Renderer(*window);

	// Clear delta time to exclude start up time
	window->GetTimer().GetTimeDeltaSeconds();
	// TODO: Way to exit from ingame, controller support
	while (window->UpdateWindow() && !NCL::Window::GetKeyboard()->KeyDown(NCL::KeyCodes::ESCAPE)) {
		float dt = window->GetTimer().GetTimeDeltaSeconds();
		// TODO: Update loop
		renderer->Update(dt);

		renderer->Render();
	}

	delete renderer;
	delete window;
}
