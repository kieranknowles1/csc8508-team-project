#include <memory>

#include <NCLCoreClasses/Window.h>
#include <NCLCoreClasses/GameTimer.h>
#include <CSC8503CoreClasses/Debug.h>

#include "TutorialGame.h"
#include "NavMesh.h"
#include "Config.h"
#include "PushdownState.h"
#include "PushdownMachine.h"
//#include "MainMenuState.h"

#include "Multiplayer/GamePacketHandlers.hpp"

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


//void TestPacketHandlers() {
//	btVector3 linear;
//	btVector3 angular;
//
//	Packet::DeltaPacketHandler deltaHandler;
//	Packet::PacketRegister::Register(&deltaHandler);
//
//	std::shared_ptr<Packet::DeltaPacket> testDeltaPacket = std::make_shared<Packet::DeltaPacket>(10, btVector3(1, 2, 3), btVector3(4, 2, 7), 5);
//	linear = testDeltaPacket->GetLinearVelocity();
//	angular = testDeltaPacket->GetAngularVelocity();
//
//
//	std::cout << "Test Delta Packet Initial Data:\n";
//	std::cout << "\tObjectID: " << testDeltaPacket->GetTargetID() << std::endl;
//	std::cout << "\tLinear Velocity: " << linear.x() << ", " << linear.y() << ", " << linear.z() << std::endl;
//	std::cout << "\tAngular Velocity: " << angular.x() << ", " << angular.y() << ", " << angular.z() << std::endl;
//	std::cout << "\tSequence Number: " << testDeltaPacket->GetSequenceNumber() << std::endl;
//
//	ENetPacket* deltaENetPacket = deltaHandler.ToENetPacket(testDeltaPacket);
//	std::shared_ptr<Packet::Packet> testDeltaPacketReturned = deltaHandler.Translate(deltaENetPacket);
//	std::shared_ptr<Packet::DeltaPacket> deltaTypeConverted = std::static_pointer_cast<Packet::DeltaPacket>(testDeltaPacketReturned);
//	linear = deltaTypeConverted->GetLinearVelocity();
//	angular = deltaTypeConverted->GetAngularVelocity();
//
//	std::cout << "Test Delta Packet Returned Data:\n";
//	std::cout << "\tObjectID: " << deltaTypeConverted->GetTargetID() << std::endl;
//	std::cout << "\tLinear Velocity: " << linear.x() << ", " << linear.y() << ", " << linear.z() << std::endl;
//	std::cout << "\tAngular Velocity: " << angular.x() << ", " << angular.y() << ", " << angular.z() << std::endl;
//	std::cout << "\tSequence Number: " << deltaTypeConverted->GetSequenceNumber() << std::endl;
//}

class MainMenuScreen : public PushdownState {
	int selection = 0;

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {

	}
};

class PauseScreen : public PushdownState {
	int selection = 0;

public:
	PauseScreen(Controller* controller) : controller(controller) {}
	Controller* controller;

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (controller->GetDigital(Controller::DigitalControl::Unpause)) {
			return PushdownResult::Pop;
		}
		Debug::Print("Press U to unpause the game!", Vector2(20, 20), Vector4(0, 0, 0, 1));
		//return PushdownResult::NoChange;
		//Debug::Print("Press U to unpause the game!", Vector2(10, 10), Vector4(0, 0, 0, 1));

		const std::string resumeGame = "Resume";
		const std::string exitGame = "Exit";

		bool inMenu = true;

		std::string menuItems[2] = { resumeGame, exitGame };

		if (inMenu) {
			if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
				selection = std::min(1, selection + 1);
			}
			if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
				selection = std::max(0, selection - 1);
			}
			if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
				const std::string pauseSelection = menuItems[selection];

				if (pauseSelection == "Resume") {
					return PushdownResult::Pop;
				} //Add an else function here to return to main menu
				inMenu = false;
			}

			for (int i = 0; i < 2; i++) {
				std::string currentItem = menuItems[i];
				if (i == selection) currentItem = currentItem + " <";
				Debug::Print(currentItem, Vector2(1, 50 + (10 * i)));
			}
		}

		return PushdownResult::NoChange;
	}
	void OnAwake() override {
	}
};

class GameScreen : public PushdownState {
public:
	GameScreen(Controller* controller) : controller(controller) {}
	Controller* controller;

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		pauseReminder -= dt;

		if (pauseReminder < 0) {
			Debug::Print("Press P to pause the game!", Vector2(20, 20), Vector4(0, 0, 0, 1));
		}
		if (controller->GetDigital(Controller::DigitalControl::Pause)) {
			pauseReminder = 0;
			*newState = new PauseScreen(controller);
			std::cout << "Game entered pause state \n";
			return PushdownResult::Push;
		}
		if (controller->GetDigital(Controller::DigitalControl::PauseQuit)) {
			Debug::Print("Going back to main menu", Vector2(0.1f, 0.3f), Vector4(0, 0, 0, 1));
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	};
	void OnAwake() override {
		std::cout << "Game state active\n";
	}
protected:
	float pauseReminder = 1;
};



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
	return new KeyboardMouseController(*window->GetKeyboard(), *window->GetMouse());
#else
	return ((PS5::PS5Window*)window)->GetController();
#endif // !__PROSPERO__

}

/*int main(int argc, char** argv) {
	auto config = Config("user-config.jsonc", "default-config.jsonc");


	auto window = createWindow(config);
	bool paused = false;

	window->ShowOSPointer(false);
	window->LockMouseToWindow(true);

	auto renderer = std::make_unique<GameTechRenderer>();
	auto controller = std::make_unique<KeyboardMouseController>(*window->GetKeyboard(), *window->GetMouse());

	auto game = std::make_unique<TutorialGame>(renderer.get(), controller.get());
	// Clear delta time to exclude start up time
	window->GetTimer().GetTimeDeltaSeconds();

	while (window->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {

		if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::P)) {
			paused = !paused;
		}

		float dt = window->GetTimer().GetTimeDeltaSeconds();

		window->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));
		if (!paused) {
			game->UpdateGame(dt);
		}

		renderer->Update(dt);
		renderer->Render();


		Debug::UpdateRenderables(dt);
	}
}*/


int main(int argc, char** argv) {
	auto config = Config("user-config.jsonc", Assets::DEFAULTCONFIG);
	bool quickStart = config.get<bool>("quickStart");

	auto window = createWindow(config);
	//bool paused = false;

	window->ShowOSPointer(false);
	window->LockMouseToWindow(true);

	auto renderer = createRenderer(window.get());
	auto controller = createController(window.get());
	PushdownMachine machine(new GameScreen(controller));


	auto game = std::make_unique<TutorialGame>(renderer.get(), controller);


	// Clear delta time to exclude start up time
	window->GetTimer().GetTimeDeltaSeconds();

	const std::string singleplayer = "Singleplayer";
	const std::string hostGame = "Host Game";
	const std::string joinGame = "Join Game";
	int selection = 0;
	bool inMenu = true;

	std::string menuItems[3] = { singleplayer, hostGame, joinGame };

	while (window->UpdateWindow() && !window->GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		float dt = window->GetTimer().GetTimeDeltaSeconds();

		if (inMenu) {

			//Try showing the FMod logo here

			if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
				selection = std::min(2, selection + 1);
			}
			if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
				selection = std::max(0, selection - 1);
			}
			if (controller->GetDigital(Controller::DigitalControl::MenuConfirm) || quickStart) {
				GameMode mode = static_cast<GameMode>(selection);

				if (mode == GameMode::SINGLEPLAYER) {
					game->LoadWorldFromFile(8);
				}
				else {
					game->JoinGame(mode == GameMode::HOST_GAME);
				}
				inMenu = false;
			}

			for (int i = 0; i < 3; i++) {
				std::string currentItem = menuItems[i];
				if (i == selection) currentItem = currentItem + " <";
				Debug::Print(currentItem, Vector2(1, 50 + (10 * i)));
			}
		}
		else {
			if (!machine.Update(dt)) {
				inMenu = true;
			}
			//Add a GetState function to PushdownMachine/PushdownState to return the screen it's on, and if it's on PauseScreen
			//Pause the game->UpdateGame
		}

		window->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		game->UpdateGame(dt);

		renderer->drawFrame(dt);


		Debug::UpdateRenderables(dt);
	}
}
