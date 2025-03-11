#pragma once

//#include "PushdownMachine.h"
#include "PushdownState.h"
#include <TutorialGame.h>
//#include "Window.h"
#include <CSC8503CoreClasses/Debug.h>
#include <iostream>
#include <GameScreen.h>

using namespace NCL;
using namespace NCL::CSC8503;

class MainMenuScreen : public PushdownState {
    int selection = 0;
    bool inMenu;
    TutorialGame* game;
    Controller* controller;
    std::string gameMode;

public:
    MainMenuScreen(Controller* controller, TutorialGame* game) : controller(controller), game(game), selection(0), inMenu(true) {}

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {

        const std::string menuItems[3] = { "Singleplayer", "Host Game", "Join Game" };

        if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
            selection = std::min(2, selection + 1);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
            selection = std::max(0, selection - 1);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
            GameMode mode = static_cast<GameMode>(selection);

            if (mode == GameMode::SINGLEPLAYER) {
                game->LoadWorldFromFile(9);
                gameMode = "Singleplayer";
            }
            else if (mode == GameMode::HOST_GAME) {
                game->JoinGame(true);
                gameMode = "Multiplayer";
            }
            /*else {
                game->JoinGame(mode == GameMode::HOST_GAME);
                gameMode = "Multiplayer";
            }*/
            *newState = new GameScreen(controller, game, gameMode);
            inMenu = false;
            return PushdownResult::Push;
        }

        //Render menu
        for (int i = 0; i < 3; i++) {
            std::string currentItem = menuItems[i];
            if (i == selection) currentItem += " <";
            Debug::Print(currentItem, Vector2(1, 50 + (10 * i))); //Replace with AddUIElement
        }
        return PushdownResult::NoChange;

        //Add FMOD Logo here as well as the line "Audio Engine: FMOD Studio by Firelight Technologies Pty Ltd."
    }

    void OnAwake() override {

    }
};