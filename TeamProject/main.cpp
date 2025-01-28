#include <stdexcept>

#include <NCLCoreClasses/Window.h>
#include <NCLCoreClasses/GameTimer.h>
#include <OpenGLRendering/OGLRenderer.h>

#include <OpenGLRendering/glad/gl.h>

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

NCL::Rendering::RendererBase* createRenderer(NCL::Window& window) {
	NCL::Rendering::OGLRenderer* renderer = new NCL::Rendering::OGLRenderer(window);
	if (!renderer->HasInitialised()) {
		throw std::runtime_error("Renderer failed to initialise!");
	}

	return renderer;
}

int main(int argc, char** argv) {
	auto window = createWindow();
	auto renderer = createRenderer(*window);

	// Clear delta time to exclude start up time
	window->GetTimer().GetTimeDeltaSeconds();
	// TODO: Way to exit from ingame, controller support
	while (window->UpdateWindow() && !NCL::Window::GetKeyboard()->KeyDown(NCL::KeyCodes::ESCAPE)) {
		float dt = window->GetTimer().GetTimeDeltaSeconds();
		// TODO: Update loop
		// Something to show that we are rendering
		glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		renderer->Update(dt);

		renderer->Render();
	}

	delete renderer;
	delete window;
}
