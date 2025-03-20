#pragma once

#include <string>
#include <algorithm>
#include "PushdownState.h"
#include "Controller.h"
#include "TutorialGame.h"
#include "Debug.h"
#include <iostream>
#include "GameScreen.h"
#include "CreditsScreen.h"

using namespace NCL;
using namespace NCL::CSC8503;


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

    void UpdateMenu(unsigned int);

private:
    UIBox background;
    std::vector<Button> buttons;
    std::vector<ButtonText> buttonTexts;
    std::vector<UIBox> leaderboardBorder;
    std::vector<UIBox> leaderboard;
    std::vector<ButtonText> leaderboardTexts;
    Vector4 activeButton = Vector4(1, 1, 1, 1);
    Vector4 inactiveButton = Vector4(0, 0, 0, 1);
    Vector4 activeText = Vector4(0, 0, 0, 1);
    Vector4 inactiveText = Vector4(1, 1, 1, 1);
};


class EndScreenMP : public PushdownState {
    unsigned int selection = 0;

public:
    EndScreenMP(Controller* controller, TutorialGame* game) : controller(controller), game(game), selection(0) {
		ui = std::make_unique<EndScreenUI>();
        renderer = game->GetUIRenderer();
		renderer->AddUiElement(ui.get());
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
            case 1:
                //Quit
            }
        }

        return PushdownResult::NoChange;
    }

    void UpdateSelection(unsigned int selection) {
		ui->UpdateMenu(selection);
    }

    void OnAwake() override {
    }

private:
	std::unique_ptr<EndScreenUI> ui;
	GameTechRendererInterface* renderer;
};