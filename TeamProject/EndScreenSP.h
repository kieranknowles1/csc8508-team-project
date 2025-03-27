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

	void SetScore(int score) {
		this->score = score;
	}

private:

    UIBox background;
    std::vector<Button> buttons;
    std::vector<Text> buttonTexts;
    Vector4 activeButton = Vector4(0.3, 0.3, 0.3, 0.5);
    Vector4 inactiveButton = Vector4(0, 0, 0, 1);
    Vector2 buttonSize = Vector2(0.4f, 0.1f);

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
    }
    Controller* controller;
    TutorialGame* game;

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {

        if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {

            game->ClearWorld();
			return PushdownResult::Clear;
        }

        return PushdownResult::NoChange;
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