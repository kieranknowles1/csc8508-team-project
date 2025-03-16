#pragma once

//#include "PushdownMachine.h"
#include "PushdownState.h"
#include <TutorialGame.h>
//#include "Window.h"
#include <CSC8503CoreClasses/Debug.h>
#include <iostream>
#include "GameScreen.h"
#include "CreditsScreen.h"

namespace NCL::CSC8503 {

class MainMenuScreen : public PushdownState {
    size_t selection = 0;
    bool inMenu;
    TutorialGame* game;
    Controller* controller;

public:
    MainMenuScreen(Controller* controller, TutorialGame* game) : controller(controller), game(game), selection(0), inMenu(true) {}

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {


        const std::array<std::string, 5> menuItems = { " Singleplayer", " Host Game", " Join Game", " Credits", " Quit"};

        if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
            selection = std::min(menuItems.size() - 1, selection + 1);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
            selection = std::max(size_t(0), selection - 1);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
            GameMode mode = static_cast<GameMode>(selection);

            switch (mode)
            {
            case GameMode::SINGLEPLAYER:
                game->Start();
                break;
            case GameMode::HOST_GAME:
                game->JoinGame(true); //This is host game
                break;
            case GameMode::JOIN_GAME:
                game->JoinGame(false); //This is join game
                break;
            case GameMode::CREDITS:
                *newState = new CreditsScreen(controller, Assets::CREDITS);
                return PushdownResult::Push;
                break;
            case GameMode::QUIT:
                return PushdownResult::Pop;
            default: assert(false);
            }

            *newState = new GameScreen(controller, game);
            inMenu = false;
            return PushdownResult::Push;
        }

        //Render menu
        for (int i = 0; i < menuItems.size(); i++) {
            std::string currentItem = menuItems[i];
            if (i == selection) currentItem = ">" + currentItem + " <";
            Debug::Print(currentItem, Vector2(0.35f, 0.35f + (0.1f * i)));
        }
        return PushdownResult::NoChange;

        //Add FMOD Logo here as well as the line "Audio Engine: FMOD Studio by Firelight Technologies Pty Ltd."
    }

    void OnAwake() override {

    }
};

}
