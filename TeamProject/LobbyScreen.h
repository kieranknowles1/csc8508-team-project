#pragma once

#include "GameScreen.h"
#include "PushdownState.h"
#include "Multiplayer/Server.hpp"

namespace NCL {
    namespace CSC8503 {
        enum class HostLobbyItems : uint8_t {
            StartGame,
            CloseLobby
        };

        enum class ClientLobbyItems : uint8_t {
            LeaveLobby
        };


        class HostLobbyScreen : public PushdownState {
        public:
            HostLobbyScreen(Controller* controller, TutorialGame* game) : controller(controller), game(game) {}

            void UpdateSelection() {
                if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
                    selection = std::min((uint8_t)menuItems.size() - 1, selection + 1);
                }
                if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
                    selection = std::max(0, selection - 1);
                }
            }

            PushdownResult OnUpdate(float dt, PushdownState** newState) override {
                UpdateSelection();

                if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
                    switch (selection) {
                    case (uint8_t) HostLobbyItems::StartGame:
                        game->Start();
                        game->GetServerInstance()->ResetTick();
                        *newState = new GameScreen(controller, game);
                        return PushdownResult::Push;
                    case (uint8_t) HostLobbyItems::CloseLobby:
                        // TODO: Close Server Packet.
                        return PushdownResult::Pop;
                    }
                }
                Render();
                return PushdownResult::NoChange;
            }

            void Render() {
                for (int i = 0; i < menuItems.size(); i++) {
                    const std::string& item = menuItems[i];
                    std::string render = i == selection ? "> " + item + " <" : "  " + item;

                    Debug::Print(render, Vector2(0.35f, 0.32f + (0.1f * i)));
                }
            }

        private:
            std::array<std::string, 2> menuItems = { "Start Game", "Close Lobby" };
            uint8_t selection = 0;
            TutorialGame* game;
            Controller* controller;
        };
        
        
        class ClientLobbyScreen : public PushdownState {
        public:
            ClientLobbyScreen(Controller* controller, TutorialGame* game) : controller(controller), game(game) {}

            void UpdateSelection() {
                if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
                    selection = std::min((uint8_t)menuItems.size() - 1, selection + 1);
                }
                if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
                    selection = std::max(0, selection - 1);
                }
            }

            PushdownResult OnUpdate(float dt, PushdownState** newState) override {
                UpdateSelection();

                if (game->GetState() == GameState::STARTING) {
                    game->Start();
                    game->GetServerInstance()->ResetTick();
                    *newState = new GameScreen(controller, game);
                    return PushdownResult::Push;
                }

                if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
                    switch (selection) {
                    case (uint8_t) ClientLobbyItems::LeaveLobby:
                        // TODO: Close Server Packet.
                        return PushdownResult::Pop;
                    }
                }
                Render();
                return PushdownResult::NoChange;
            }

            void Render() {
                for (int i = 0; i < menuItems.size(); i++) {
                    const std::string& item = menuItems[i];
                    std::string render = i == selection ? "> " + item + " <" : "  " + item;

                    Debug::Print(render, Vector2(0.35f, 0.32f + (0.1f * i)));
                }
            }

        private:
            std::array<std::string, 1> menuItems = { "Leave Lobby" };
            uint8_t selection = 0;
            TutorialGame* game;
            Controller* controller;
        };
    }
}
