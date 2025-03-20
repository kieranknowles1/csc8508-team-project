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

    void UpdateMenu(int);

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
    //unsigned int selection = 0;
};


class EndScreenMP : public PushdownState {
    size_t selection = 0;

public:
    EndScreenMP(Controller* controller, TutorialGame* game) : controller(controller), game(game), selection(0) {
		ui = std::make_unique<EndScreenUI>();
        renderer = game->GetUIRenderer();
		renderer->AddUiElement(ui.get());
		ui->SetActive(true);
    }
    Controller* controller;
    TutorialGame* game;

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {

        const std::array<std::string, 2> menuItems = { "Main Menu", "Quit" };
        
        if (controller->GetDigital(Controller::DigitalControl::MenuRight)) {
            selection = std::min(menuItems.size() - 1, selection + 1);
            UpdateSelection(selection);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuLeft)) {
            selection = std::max(size_t(0), selection - 1);
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
	GameTechRendererInterface* renderer;
};