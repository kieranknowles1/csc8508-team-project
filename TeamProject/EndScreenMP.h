#pragma once

#include <string>
#include <algorithm>
#include "PushdownState.h"
#include "Controller.h"
#include "TutorialGame.h"
#include "Debug.h"

using namespace NCL;
using namespace NCL::CSC8503;



class EndScreenMP : public PushdownState {
    int selection = 0;

public:
    EndScreenMP(Controller* controller, TutorialGame* game) : controller(controller), game(game) {}
    Controller* controller;
    TutorialGame* game;

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {

		std::string menuItems[2] = { "Main Menu", "Quit" };
        if (controller->GetDigital(Controller::DigitalControl::MenuRight)) {
            selection = std::min(1, selection + 1);
            UpdateSelection();
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuLeft)) {
            selection = std::max(0, selection - 1);
			UpdateSelection();
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
            const std::string menuSelection = menuItems[selection];

            if (menuSelection == "Quit") {
                game->SetGameMode(GameMode::QUIT);
                return PushdownResult::Clear;
            }
            else {
                return PushdownResult::Clear;
            }
        }

        return PushdownResult::NoChange;
    }

    void UpdateSelection();

    void OnAwake() override {
    }
};

class EndScreenUI : public UiElement {
public:
    EndScreen();
    void render(std::vector<UiSprite>& sprites) override;
    void render(std::vector<UiText>& texts) override;
};
