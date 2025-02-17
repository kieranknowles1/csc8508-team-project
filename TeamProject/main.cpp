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
	NavMesh navMesh;
	if (navMesh.LoadFromFile("C:\\GitHub\\csc8508-team-project\\Assets\\Meshes\\NavMeshes\\smalltest.navmesh")) {
		btVector3 start(40, 0, -25);
		btVector3 end(45, 0, -30);
		std::vector<btVector3> path = navMesh.FindPath(start, end);

		if (!path.empty()) {
			std::cout << "Path found! Points: " << path.size() << std::endl;
			for (const auto& point : path) {
				std::cout << "-> (" << point.x() << ", " << point.y() << ", " << point.z() << ")\n";
			}
		}
		else {
			std::cout << "No valid path!" << std::endl;
		}
	}
	return 0;

	/*auto window = createWindow();

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
	delete window;*/
}
