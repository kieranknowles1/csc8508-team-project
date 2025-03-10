//#include "PushdownMachine.h"
#include "PushdownState.h"
#include <TutorialGame.h>
#include "Window.h"
#include <CSC8503CoreClasses/Debug.h>
#include <iostream>

using namespace NCL;
using namespace NCL::CSC8503;

class MainMenuScreen : public PushdownState {
public:
	MainMenuScreen(TutorialGame* game) : game(game), selection(0), inMenu(true) {}

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		//if (!inMenu) return PushdownResult::Push; // Move to GameScreen when a mode is selected

		const std::string menuItems[3] = { "Singleplayer", "Host Game", "Join Game" };

		// Handle input
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::DOWN)) {
			selection = std::min(2, selection + 1);
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::UP)) {
			selection = std::max(0, selection - 1);
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::RETURN)) {
			GameMode mode = static_cast<GameMode>(selection);

			if (mode == GameMode::SINGLEPLAYER) {
				game->LoadWorldFromFile(8);
			}
			else {
				game->JoinGame(mode == GameMode::HOST_GAME);
			}

			inMenu = false;  // Hide menu once a game mode is selected
			*newState = new GameScreen();  // Push the game state
			return PushdownResult::Push;
		}

		// Render menu
		for (int i = 0; i < 3; i++) {
			std::string currentItem = menuItems[i];
			if (i == selection) currentItem += " <";
			Debug::Print(currentItem, Vector2(1, 50 + (10 * i)));
		}

		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		std::cout << "Main Menu Active\n";
		inMenu = true;
	}
	
protected:
	TutorialGame* game;
	int selection;
	bool inMenu;
};