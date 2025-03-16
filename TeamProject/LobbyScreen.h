#pragma once

#include "GameScreen.h"
#include "PushdownState.h"

namespace NCL {
    namespace CSC8503 {
        class LobbyScreen : public PushdownState {
        public:
            LobbyScreen(Controller* controller, TutorialGame* game) : controller(controller), game(game) {}

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
                    game->Start();
                    *newState = new GameScreen(controller, game);
                    return PushdownResult::Push;
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
            std::array<std::string, 1> menuItems = { "Start Game" };
            uint8_t selection = 0;
            TutorialGame* game;
            Controller* controller;
        };
    }
}
