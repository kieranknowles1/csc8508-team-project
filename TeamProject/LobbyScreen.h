#pragma once

#include "PushdownState.h"
#include "TutorialGame.h"
#include <CSC8503CoreClasses/Debug.h>
#include <NCLCoreClasses/Window.h>
#include "Colors.h"
#include <vector>
#include <unordered_map>
#include "AudioEngine.h"
#include "Multiplayer/Server.hpp"

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
			if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
				if (selection < 1) {
					selection++;
					int channelId = audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), -12.0f);
					audioEngine.SetChannelPitchMultiplier(channelId, 0.9f);
				}
			}

			if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
				if (selection > 0) {
					selection--;
					audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), -12.0f);
				}
			}
		}

		PushdownResult OnUpdate(float dt, PushdownState** newState) override {
			UpdateSelection();

			if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
				audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);

				if (selection == 0) {
					// START GAME
					game->Start();
					game->GetServerInstance()->ResetTick();
					*newState = new MultiplayerGameScreen(controller, game, game->GetPlayerController());
					return PushdownResult::Push;
				}
				else if (selection == 1) {
					// CLOSE LOBBY
					game->StopServer();
					return PushdownResult::Pop;
				}

				else if (selection == 9) {  // Start Button (only host can press)
					//TODO: Let me know if this sound cuts off abruptly
					audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
                    game->Start();
                    game->GetServerInstance()->ResetTick();
                    *newState = new MultiplayerGameScreen(controller, game, game->GetPlayerController());
                    return PushdownResult::Push;
				}

			}

			RenderUI();
			return PushdownResult::NoChange;
		}

		void RenderUI() {
			Debug::Print("LOBBY", Vector2(0.42f, 0.1f));

			Vector2 leftStartPos = Vector2(0.3f, 0.3f);
			Vector2 rightStartPos = Vector2(0.5f, 0.3f);
			float verticalSpacing = 0.09f;

			Vector2 posStart[2] = { leftStartPos, rightStartPos };
			Vector2 youOffset[2] = { Vector2(-0.1, 0), Vector2(0.1425, 0) };

			for (int i = 0; i < 8; ++i) {
				std::string displayName = "Empty";
				std::string you = (i / 4) == 0 ? "YOU > " : " < YOU";
				Vector2 pos = posStart[i / 4] + Vector2(0, (i % 4) * verticalSpacing);
				Vector4 color = Color::GetPlayerColor(i);

				if (i < game->GetServerInstance()->ClientCount()) {
					displayName = "Player " + std::to_string(i + 1);

					Lobbies::User* user = game->GetServerInstance()->GetUser();
					if (user && i + 1 == user->GetUserID()) {
                        Debug::Print(you, pos + youOffset[i / 4], color);
					}
				}
				Debug::Print(displayName, pos, color);
			}

			Vector2 buttonStart = Vector2(0.35f, 0.85f);
			Vector2 spacing = Vector2(0, 0.09f);

			std::string startLabel = (selection == 0) ? "> START GAME <" : "  START GAME";
			std::string closeLabel = (selection == 1) ? "> CLOSE LOBBY <" : "  CLOSE LOBBY";

			Debug::Print(startLabel, buttonStart);
			Debug::Print(closeLabel, buttonStart + spacing);
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

		PushdownResult OnUpdate(float dt, PushdownState** newState) override {

            if (game->GetState() == GameState::STARTING) {
                game->Start();
                game->GetServerInstance()->ResetTick();
                game->SetState(GameState::ACTIVE);
                *newState = new MultiplayerGameScreen(controller, game, game->GetPlayerController());
                return PushdownResult::Push;
            }

			if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
				audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
			    game->StopServer();
				return PushdownResult::Pop;
			}

			RenderUI();
			return PushdownResult::NoChange;
		}

		void RenderUI() {
			Debug::Print("LOBBY", Vector2(0.42f, 0.1f));

			Vector2 leftStartPos = Vector2(0.3f, 0.3f);
			Vector2 rightStartPos = Vector2(0.5f, 0.3f);
			float verticalSpacing = 0.09f;

			Vector2 posStart[2] = { leftStartPos, rightStartPos };
			Vector2 youOffset[2] = { Vector2(-0.1, 0), Vector2(0.1425, 0) };

			for (int i = 0; i < 8; ++i) {
				std::string displayName = "Empty";
				std::string you = (i / 4) == 0 ? "YOU > " : " < YOU";
				Vector2 pos = posStart[i / 4] + Vector2(0, (i % 4) * verticalSpacing);
				Vector4 color = Color::GetPlayerColor(i);

				if (i < game->GetServerInstance()->ClientCount()) {
					displayName = "Player " + std::to_string(i + 1);

					Lobbies::User* user = game->GetServerInstance()->GetUser();
					if (user && i + 1 == user->GetUserID()) {
                        Debug::Print(you, pos + youOffset[i / 4], color);
					}
				}
				Debug::Print(displayName, pos, color);
			}

			Vector2 buttonStart = Vector2(0.35f, 0.85f);

			std::string leaveLabel = "> LEAVE LOBBY <";

			Debug::Print(leaveLabel, buttonStart);
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
}
