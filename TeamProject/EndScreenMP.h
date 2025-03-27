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


class EndScreenMPUI : public UiElement {

public:
    EndScreenMPUI();

    void render(std::vector<UiSprite>& sprites) override;

    void render(std::vector<UiText>& texts) override;

    void Animate(float dt) override {};

    void InitMenu();

    void AddPlayer(Player playerData) {
        players.push_back(playerData);
    }

private:
	void PopulateLeaderboard();
    
    UIBox background;
    std::vector<Button> buttons;
    std::vector<Text> buttonTexts;
    Vector4 activeButton = Vector4(0.3, 0.3, 0.3, 0.5);
    Vector4 inactiveButton = Vector4(0, 0, 0, 1);
    Vector4 textColor = Vector4(0, 0, 0, 1);

    Vector4 backgroundColor = Vector4(0, 0, 0, 1);
    Vector4 buttonColor = Vector4(0.4f, 0.4f, 0.4f, 1);
    Vector4 borderColor = Vector4(0, 0, 0, 1);
    Vector4 boxColor = Vector4(0.4f, 0.4f, 0.4f, 1);
    float borderThickness = 0.005f;
    Vector2 buttonSize = Vector2(0.4f, 1);

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
		ui = std::make_unique<EndScreenMPUI>();
        renderer = game->GetUIRenderer();
		renderer->AddUiElement(ui.get());
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

    void OnAwake() override {
    }

private:
	std::unique_ptr<EndScreenMPUI> ui;
	GameTechRendererInterface* renderer;
};