#include <memory>

#include <NCLCoreClasses/Window.h>
#include <NCLCoreClasses/GameTimer.h>
#include <CSC8503CoreClasses/Debug.h>

#include "GameTechRenderer.h"
#include "TutorialGame.h"
#include "NavMesh.h"
#include "Config.h"
#include "PushdownState.h"
#include "PushdownMachine.h"
//#include "MainMenuState.h"

#include "Multiplayer/GamePacketHandlers.hpp"

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

class PauseScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::U)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
		Debug::Print("Press U to unpause the game!", Vector2(10, 10), Vector4(0, 0, 0, 1));
	}
	void OnAwake() override {
	}
};

class GameScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		pauseReminder -= dt;

		if (pauseReminder < 0) {
			Debug::Print("Press P to pause the game!", Vector2(0.1f, 0.1f), Vector4(0, 0, 0, 1));
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::P)) {
			pauseReminder = 0;
			*newState = new PauseScreen();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::RCONTROL)) {
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
	WindowInitialisation options = {
		.width = config.get<uint32_t>("windowWidth"),
		.height = config.get<uint32_t>("windowHeight"),
		.fullScreen = config.get<FullScreenState>("fullscreen"),
		.windowTitle = "Team Project",
	};

	std::unique_ptr<Window> window(Window::CreateGameWindow(options));
	if (!window || !window->HasInitialised()) {
		throw std::runtime_error("Window failed to initialise!");
	}

	return window;
}

/*int main(int argc, char** argv) {
	auto config = Config("user-config.jsonc", "default-config.jsonc");


	auto window = createWindow(config);
	bool paused = false;

	window->ShowOSPointer(false);
	window->LockMouseToWindow(true);

	auto world = std::make_unique<GameWorld>();
	auto renderer = std::make_unique<GameTechRenderer>(world.get());
	auto controller = std::make_unique<KeyboardMouseController>(*window->GetKeyboard(), *window->GetMouse());

	auto game = std::make_unique<TutorialGame>(renderer.get(), world.get(), controller.get());
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
	PushdownMachine machine(new GameScreen());

	auto config = Config("user-config.jsonc", "default-config.jsonc");


	auto window = createWindow(config);
	//bool paused = false;

	window->ShowOSPointer(false);
	window->LockMouseToWindow(true);

	auto world = std::make_unique<GameWorld>();
	auto renderer = std::make_unique<GameTechRenderer>(world.get());
	auto controller = std::make_unique<KeyboardMouseController>(*window->GetKeyboard(), *window->GetMouse());

	auto game = std::make_unique<TutorialGame>(renderer.get(), world.get(), controller.get());
	// Clear delta time to exclude start up time
	window->GetTimer().GetTimeDeltaSeconds();

	while (window->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {

		//if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::P)) {
		//	paused = !paused;
		//}

		float dt = window->GetTimer().GetTimeDeltaSeconds();

		window->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));
		//if (!paused) {
		//	game->UpdateGame(dt);
		//}
		game->UpdateGame(dt);
		//machine.Update(dt);

		renderer->Update(dt);
		renderer->Render();
		machine.Update(dt);


		Debug::UpdateRenderables(dt);
	}
}
