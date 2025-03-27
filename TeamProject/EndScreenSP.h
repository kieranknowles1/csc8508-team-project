#pragma once

#include <string>
#include <algorithm>
#include "PushdownState.h"
#include "Controller.h"
#include "TutorialGame.h"
#include "Debug.h"
#include <iostream>
#include "CreditsScreen.h"

using namespace NCL;
using namespace NCL::CSC8503;

class EndScreenSPUI : public UiElement {

public:
    EndScreenSPUI();

    void render(std::vector<UiSprite>& sprites) override;

    void render(std::vector<UiText>& texts) override;

    void Animate(float dt) override {};

    void InitMenu();

    void UpdateMenu(unsigned int);

	void SetScore(int score) {
		this->score = score;
	}

private:

    UIBox background;
    std::vector<Button> buttons;
    std::vector<Text> buttonTexts;
    Vector4 activeButton = Vector4(0.3, 0.3, 0.3, 0.5);
    Vector4 inactiveButton = Vector4(0, 0, 0, 1);
    Vector2 buttonSize = Vector2(0.3f, 0.075f);

    Vector4 backgroundColor = Vector4(0, 0, 0, 1);
    Vector4 buttonColor = Vector4(0.4f, 0.4f, 0.4f, 1);
    Vector4 textColor = Vector4(0, 0, 0, 1);

	int score;
};


class EndScreenSP : public PushdownState {
    unsigned int selection = 0;

public:
    EndScreenSP(Controller* controller, TutorialGame* game, int s) : controller(controller), game(game), selection(0), score(s) {
        ui = std::make_unique<EndScreenSPUI>();
        renderer = game->GetUIRenderer();
        renderer->AddUiElement(ui.get());
        ui->SetScore(score);
        ui->SetActive(true);
        ui->UpdateMenu(selection);
    }
    Controller* controller;
    TutorialGame* game;

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {

        if (controller->GetDigital(Controller::DigitalControl::MenuRight)) {
            selection = 1;
            UpdateSelection(selection);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuLeft)) {
            selection = 0;
            UpdateSelection(selection);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {

            switch (selection)
            {
            case 0:
                //Main Menu
                break;
            case 1:
                //Quit
                break;
            default:
                break;
            }
        }

        return PushdownResult::NoChange;
    }

    void UpdateSelection(unsigned int selection) {
        ui->UpdateMenu(selection);
    }

	void SetScore(int score) {
		score = score;
	}

    void OnAwake() override {
    }

private:
    std::unique_ptr<EndScreenSPUI> ui;
    GameTechRendererInterface* renderer;
	int score = 0;
};