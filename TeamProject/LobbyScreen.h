#pragma once

//#include "TeamProject/Multiplayer/Lobby.hpp"
//#include "TeamProject/Multiplayer/User.hpp"

#include "PushdownState.h"
#include "TutorialGame.h"
#include <CSC8503CoreClasses/Debug.h>
#include <NCLCoreClasses/Window.h>
#include "Colors.h"
#include <vector>
#include <unordered_map>
#include "AudioEngine.h"

namespace NCL::CSC8503 {

	enum class HostLobbyItems : uint8_t {
		StartGame,
		CloseLobby
	};

	enum class ClientLobbyItems : uint8_t {
		LeaveLobby
	};


	class HostLobbyScreen : public PushdownState {
	public: 
		HostLobbyScreen(Controller* controller, TutorialGame* game) : controller(controller), game(game), selection(0), playerList(8, "Empty") {
			colourTaken.fill(false);
		}

		void UpdateSelection() {
			size_t startSelection = selection;

			// Check if all colors are taken
			bool allColorsTaken = std::all_of(colourTaken.begin(), colourTaken.end(), [](bool taken) { return taken; });

			if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
				do {
					if (selection == menuItems.size() - 1) break; // Prevent going out of bounds
					selection = std::min(menuItems.size() - 1, selection + 1);
				} while (selection < 8 && colourTaken[selection]); // Skip taken colors

				if (selection != startSelection) {
					int channelId = audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), -12.0f);
					audioEngine.SetChannelPitchMultiplier(channelId, 0.9f);
				}
			}

			if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
				do {
					if (selection == 0) break; // Prevent going out of bounds
					selection = selection > 0 ? selection - 1 : selection;
				} while (selection < 8 && colourTaken[selection]); // Skip taken colors

				//Prevent Cursor from Moving to the Top if All Colors Above are Taken**
				if (selection == 0 && colourTaken[selection]) {
					selection = startSelection; // Stay in place if top color is taken
				}

				if (selection != startSelection) {
					audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), -12.0f);
				}
			}

			//Prevent moving back to colors if all are taken**
			if (allColorsTaken && selection < 8) {
				selection = 8; // Force cursor to "Close Lobby"
			}
		}

		PushdownResult OnUpdate(float dt, PushdownState** newState) override {
			UpdateSelection();

			if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
				if (selection < 8) {  // If a colour is selected
					if (!colourTaken[selection]) {
						colourTaken[selection] = true;
						for (auto& player : playerList) {
							if (player == "Empty") {
								player = menuItems[selection]; // Assign colour to first empty slot
								audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
								break;
							}
						}
					}
				}
				else if (selection == 8) {  // Close lobby button
					audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
					return PushdownResult::Pop;
				}
				else if (selection == 9) {  // Start Button (only host can press)
					//TODO: Let me know if this sound cuts off abruptly
					audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
					//game->Start();
					//*newState = new GameScreen(controller, game);
					//return PushdownResult::Push;
				}
			}

			RenderUI();
			return PushdownResult::NoChange;
		}

		void RenderUI() {
			Vector2 basePos = Vector2(0.01f, 0.2f);
			Vector2 listPos = Vector2(0.8f, 0.2f);

			Debug::Print("Select your colour!", Vector2(0.3f, 0.1f));

			// Render Colour Options
			for (size_t i = 0; i < 10; i++) {
				std::string currentItem = (i == selection) ? "> " + std::to_string(i + 1) + ". " + (i < 8 && colourTaken[i] ? "Taken" : menuItems[i]) + " <"
					: "  " + std::to_string(i + 1) + ". " + (i < 8 && colourTaken[i] ? "" : menuItems[i]);

				//Close lobby button
				if (i == 8) {
					currentItem = (i == selection) ? "> CLOSE LOBBY <" : "  CLOSE LOBBY";
					Debug::Print(currentItem, basePos + Vector2(0, 0.07f * i));
					continue;
				}

				//Start button
				if (i == 9) {
					currentItem = (i == selection) ? "> START GAME <" : "  START GAME";
					Debug::Print(currentItem, basePos + Vector2(0.35f, 0.08f * i));
					continue;
				}

				btVector4 btColour = Color::GetPlayerColor(i + 1);
				Vector4 textColour(btColour.x(), btColour.y(), btColour.z(), btColour.w());
				Debug::Print(currentItem, basePos + Vector2(0, 0.06f * i), textColour);
			}

			// Render Player List
			Debug::Print("PLAYERS", listPos);
			for (size_t i = 0; i < playerList.size(); i++) {
				Debug::Print(std::to_string(i + 1) + ". " + playerList[i], listPos + Vector2(0, 0.05f * (i + 1)));
			}
		}

	protected:
		TutorialGame* game;
		Controller* controller;
		size_t selection;
		GameTechRendererInterface* renderer;
		const std::array<std::string, 10> menuItems = { "Red", "Orange", "Blue", "Green", "Purple", "Pink", "Yellow", "Cyan", "Leave", "Start" };

		std::unordered_map<int, bool> assignedColors;
		std::vector<std::string> playerList;
		std::array<bool, 8> colourTaken;

		void AssignColour(int colourIndex);
		void StartGame(PushdownState** newState);
	};


	class ClientLobbyScreen : public PushdownState {
	public:
		ClientLobbyScreen(Controller* controller, TutorialGame* game) : controller(controller), game(game), selection(0), playerList(8, "Empty") {
			colourTaken.fill(false);
		}

		void UpdateSelection() {
			size_t startSelection = selection;

			// Check if all colors are taken
			bool allColorsTaken = std::all_of(colourTaken.begin(), colourTaken.end(), [](bool taken) { return taken; });

			if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
				do {
					if (selection == menuItems.size() - 1) break; // Prevent going out of bounds
					selection = std::min(menuItems.size() - 1, selection + 1);
				} while (selection < 8 && colourTaken[selection]); // Skip taken colors

				if (selection != startSelection) {
					int channelId = audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), -12.0f);
					audioEngine.SetChannelPitchMultiplier(channelId, 0.9f);
				}
			}

			if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
				do {
					if (selection == 0) break; // Prevent going out of bounds
					selection = selection > 0 ? selection - 1 : selection;
				} while (selection < 8 && colourTaken[selection]); // Skip taken colors

				//Prevent Cursor from Moving to the Top if All Colors Above are Taken**
				if (selection == 0 && colourTaken[selection]) {
					selection = startSelection; // Stay in place if top color is taken
				}

				if (selection != startSelection) {
					audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), -12.0f);
				}
			}

			//Prevent moving back to colors if all are taken**
			if (allColorsTaken && selection < 8) {
				selection = 8; // Force cursor to "Close Lobby"
			}
		}

		PushdownResult OnUpdate(float dt, PushdownState** newState) override {
			UpdateSelection();

			if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
				if (selection < 8) {  // If a colour is selected
					if (!colourTaken[selection]) {
						colourTaken[selection] = true;
						for (auto& player : playerList) {
							if (player == "Empty") {
								player = menuItems[selection]; // Assign colour to first empty slot
								audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
								break;
							}
						}
					}
				}
				else if (selection == 8) {  // Close lobby button
					audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
					return PushdownResult::Pop;
				}
			}

			RenderUI();
			return PushdownResult::NoChange;
		}

		void RenderUI() {
			Vector2 basePos = Vector2(0.01f, 0.2f);
			Vector2 listPos = Vector2(0.8f, 0.2f);

			Debug::Print("Select your colour!", Vector2(0.3f, 0.1f));

			// Render Colour Options
			for (size_t i = 0; i < 9; i++) {
				std::string currentItem = (i == selection) ? "> " + std::to_string(i + 1) + ". " + (i < 8 && colourTaken[i] ? "Taken" : menuItems[i]) + " <"
					: "  " + std::to_string(i + 1) + ". " + (i < 8 && colourTaken[i] ? "" : menuItems[i]);

				//Leave button
				if (i == 8) {
					currentItem = (i == selection) ? "> LEAVE <" : "  LEAVE";
					Debug::Print(currentItem, basePos + Vector2(0, 0.07f * i));
					continue;
				}

				btVector4 btColour = Color::GetPlayerColor(i + 1);
				Vector4 textColour(btColour.x(), btColour.y(), btColour.z(), btColour.w());
				Debug::Print(currentItem, basePos + Vector2(0, 0.06f * i), textColour);
			}

			// Render Player List
			Debug::Print("PLAYERS", listPos);
			for (size_t i = 0; i < playerList.size(); i++) {
				Debug::Print(std::to_string(i + 1) + ". " + playerList[i], listPos + Vector2(0, 0.05f * (i + 1)));
			}
		}

	protected:
		TutorialGame* game;
		Controller* controller;
		size_t selection;
		GameTechRendererInterface* renderer;
		const std::array<std::string, 9> menuItems = { "Red", "Orange", "Blue", "Green", "Purple", "Pink", "Yellow", "Cyan", "Leave"};

		std::unordered_map<int, bool> assignedColors;
		std::vector<std::string> playerList;
		std::array<bool, 8> colourTaken;

		void AssignColour(int colourIndex);
		void StartGame(PushdownState** newState);


	};














	class LobbyScreen : public PushdownState {
	public:
		LobbyScreen(Controller* controller, TutorialGame* game, bool isHost) : controller(controller), game(game), isHost(isHost), selection(0), playerList(8, "Empty") {

			//Initialize colour slots as available
			//for (int i = 0; i < 8; i++) {
			//	assignedColors[i] = false;
			//}
			colourTaken.fill(false);

		}

		PushdownResult OnUpdate(float dt, PushdownState** newState) override {
			//const std::array<std::string, 10> menuItems = { "Red", "Orange", "Green", "Purple", "Yellow", "Blue", "Pink", "Cyan", "Leave", "Start" };

			if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
				size_t startSelection = selection;
				do {
					selection = std::min(menuItems.size() - 1, selection + 1);
				} while (selection < 8 && colourTaken[selection] && selection != startSelection); // Skip taken colours
			}

			if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
				size_t startSelection = selection;
				do {
					selection = selection > 0 ? selection - 1 : selection;
				} while (selection < 8 && colourTaken[selection] && selection != startSelection); // Skip taken colours
			}

			//Old colour menu scrolling
			/*if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
				selection = std::min(menuItems.size() - 1, selection + 1);
			}
			if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
				selection = std::max(size_t(0), selection - 1);
			}*/

			if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
				if (selection < 8) {  // If a colour is selected
					if (!colourTaken[selection]) {
						colourTaken[selection] = true;
						for (auto& player : playerList) {
							if (player == "Empty") {
								player = menuItems[selection]; // Assign colour to first empty slot
								break;
							}
						}
					}
				}
				else if (selection == 8) {  // Leave Button
					return PushdownResult::Pop;
				}
				else if (selection == 9 && isHost) {  // Start Button (only host can press)
					game->Start();
					*newState = new GameScreen(controller, game, game->GetPlayerController());
					return PushdownResult::Push;
				}
			}

			RenderUI();
			return PushdownResult::NoChange;
		}

		void RenderUI() {
			Vector2 basePos = Vector2(0.01f, 0.2f);
			Vector2 listPos = Vector2(0.8f, 0.2f);

			Debug::Print("Select your colour!", Vector2(0.3f, 0.1f));

			// Render Colour Options
			for (size_t i = 0; i < 10; i++) {
				std::string currentItem = (i == selection) ? "> " + std::to_string(i + 1) + ". " + (i < 8 && colourTaken[i] ? "Taken" : menuItems[i]) + " <"
					: "  " + std::to_string(i + 1) + ". " + (i < 8 && colourTaken[i] ? "" : menuItems[i]);

				//Leave button
				if (i == 8) {
					currentItem = (i == selection) ? "> LEAVE <" : "  LEAVE";
					Debug::Print(currentItem, basePos + Vector2(0, 0.07f * i));
					continue;
				}

				//Start button
				if (i == 9) {
					currentItem = (i == selection) ? "> START GAME <" : "  START GAME";
					Debug::Print(currentItem, basePos + Vector2(0.35f, 0.08f * i));
					continue;
				}

				btVector4 btColour = Color::GetPlayerColor(i + 1);
				Vector4 textColour(btColour.x(), btColour.y(), btColour.z(), btColour.w());
				Debug::Print(currentItem, basePos + Vector2(0, 0.06f * i), textColour);
			}

			// Render Player List
			Debug::Print("PLAYERS", listPos);
			for (size_t i = 0; i < playerList.size(); i++) {
				Debug::Print(std::to_string(i + 1) + ". " + playerList[i], listPos + Vector2(0, 0.05f * (i + 1)));
			}
		}

	protected:
		TutorialGame* game;
		Controller* controller;
		bool isHost;
		size_t selection;
		GameTechRendererInterface* renderer;
		const std::array<std::string, 10> menuItems = { "Red", "Orange", "Blue", "Green", "Purple", "Pink", "Yellow", "Cyan", "Leave", "Start" };

		std::unordered_map<int, bool> assignedColors;
		std::vector<std::string> playerList;
		std::array<bool, 8> colourTaken;

		void AssignColour(int colourIndex);
		void StartGame(PushdownState** newState);
		//void RenderUI();
	};
}