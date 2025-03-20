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
    EndScreenMP(Controller* controller, TutorialGame* game) : controller(controller), game(game) {
		ui = std::make_unique<EndScreenUI>();
    }
    Controller* controller;
    TutorialGame* game;

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {

		std::string menuItems[2] = { "Main Menu", "Quit" };
        if (controller->GetDigital(Controller::DigitalControl::MenuRight)) {
            selection = std::min(1, selection + 1);
            UpdateSelection(selection);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuLeft)) {
            selection = std::max(0, selection - 1);
			UpdateSelection(selection);
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

    void UpdateSelection(int selection) {
		ui->UpdateMenu(selection);
    }

    void OnAwake() override {
    }

private:
	std::unique_ptr<EndScreenUI> ui;
};

struct UIBox {
    Vector2 position;
    Vector2 size;
};

struct Button {
    Vector2 position;
    Vector2 size;
    Vector4 color = Vector4(0.4f, 0.4f, 0.4f, 1);
};

struct ButtonText {
    Vector2 position;
    std::string text;
    Vector4 color = Vector4(1, 1, 1, 1);
};

struct text {
    Vector2 position;
    std::string text;
};

class EndScreenUI : public UiElement {

public:
    EndScreenUI();

    void render(std::vector<UiSprite>& sprites) override;

    void render(std::vector<UiText>& texts) override;

    void Animate(float dt) override {};

    void InitMenu();

    void UpdateMenu(int);

private:
    UIBox background;
    std::vector<Button> buttons;
    std::vector<ButtonText> buttonTexts;
    std::vector<UIBox> leaderboardBorder;
    std::vector<UIBox> leaderboard;
    std::vector<ButtonText> leaderboardTexts;
    //unsigned int selection = 0;
};