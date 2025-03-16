#pragma once

//#include "PushdownMachine.h"
#include "PushdownState.h"
#include <TutorialGame.h>
//#include "Window.h"
#include <CSC8503CoreClasses/Debug.h>
#include <iostream>
#include "GameScreen.h"
#include "LobbyScreen.h"
#include "CreditsScreen.h"

namespace NCL::CSC8503 {

class MainMenuScreen : public PushdownState {
    size_t selection = 0;
    TutorialGame* game;
    Controller* controller;

public:
    MainMenuScreen(Controller* controller, TutorialGame* game) : controller(controller), game(game), selection(0) {}

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {
        const std::array<std::string, 5> menuItems = { "Singleplayer", "Host Game", "Join Game", "Credits", "Quit"};
        
        if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
            selection = (size_t)std::min((int)menuItems.size() - 1, (int)selection + 1);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
            selection = (size_t)std::max(0, (int)selection - 1);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
            GameMode mode = static_cast<GameMode>(selection);

            switch (mode)
            {
            case GameMode::SINGLEPLAYER:
                game->Start();
                *newState = new GameScreen(controller, game);
                return PushdownResult::Push;

            case GameMode::HOST_GAME:
                game->StartMultiplayerGame(true); 
                *newState = new HostLobbyScreen(controller, game);
                return PushdownResult::Push;

            case GameMode::JOIN_GAME:
                game->StartMultiplayerGame(true); 
                *newState = new ClientLobbyScreen(controller, game);
                return PushdownResult::Push;

            case GameMode::CREDITS:
                *newState = new CreditsScreen(controller, Assets::CREDITS);
                return PushdownResult::Push;
                break;
            case GameMode::QUIT:
                return PushdownResult::Pop;
            default: assert(false);
            }
        }

        //Render menu
        for (int i = 0; i < menuItems.size(); i++) {
            std::string currentItem = menuItems[i];
            if (i == (int) selection) {
                currentItem = "> " + currentItem + " <";
            }
            else {
                currentItem = "  " + currentItem;
            }

            Debug::Print(currentItem, Vector2(0.35f, 0.32f + (0.1f * i)));
        }
        return PushdownResult::NoChange;

        //Add FMOD Logo here as well as the line "Audio Engine: FMOD Studio by Firelight Technologies Pty Ltd."
    }

    void OnAwake() override {

    }
};

}
