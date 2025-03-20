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

struct Text {
    Vector2 position;
    std::string text;
    Vector4 color = Vector4(1, 1, 1, 1);
};

class EndScreenUI : public UiElement {

public:
    EndScreenUI();

    void render(std::vector<UiSprite>& sprites) override;

    void render(std::vector<UiText>& texts) override;

    void Animate(float dt) override {};

    void InitMenu();

    void UpdateMenu(unsigned int);

    void AddPlayer(Player playerData) {
        players.push_back(playerData);
    }

private:
	void PopulateLeaderboard();
    
    UIBox background;
    std::vector<Button> buttons;
    std::vector<Text> buttonTexts;
    Vector4 activeButton = Vector4(1, 1, 1, 1);
    Vector4 inactiveButton = Vector4(0, 0, 0, 1);
    Vector4 activeText = Vector4(0, 0, 0, 1);
    Vector4 inactiveText = Vector4(1, 1, 1, 1);

    Vector2 leaderboardCenter = Vector2(0.5f, 0.6f);
    Vector2 leaderboardSize = Vector2(0.7f, 0.7f);
    std::array<ScoreboardBoxes, 27> boxes;
    std::array<Text, 27> leaderboardTexts;
    std::vector<Player> players;
    int columns = 3;
    int rows = 9;

    std::string TeamColorToString(TeamColor color) {
        switch (color) {
        case TeamColor::RED: return "Red";
        case TeamColor::BLUE: return "Blue";
        case TeamColor::GREEN: return "Green";
        case TeamColor::YELLOW: return "Yellow";
        case TeamColor::ORANGE: return "Orange";
        case TeamColor::PURPLE: return "Purple";
        case TeamColor::PINK: return "Pink";
        case TeamColor::CYAN: return "Cyan";
        default: return "Unknown";
        }
    }

    void SortPlayers() {
        std::sort(players.begin(), players.end(), [](const Player& a, const Player& b) {
            return a.score > b.score;
            });
    }
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

    void OnAwake() override {
    }

private:
	std::unique_ptr<EndScreenUI> ui;
	GameTechRendererInterface* renderer;
};