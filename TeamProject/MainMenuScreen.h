#pragma once

//#include "PushdownMachine.h"
#include "PushdownState.h"
#include <TutorialGame.h>
//#include "Window.h"
#include <CSC8503CoreClasses/Debug.h>
#include <iostream>
#include "GameScreen.h"
#include "CreditsScreen.h"
#include "GameTechRendererInterface.h"
#include "LobbyScreen.h"


namespace NCL::CSC8503 {

    enum class GameMode {
        SINGLEPLAYER,
        HOST_GAME,
        JOIN_GAME,
        CREDITS,
        QUIT,
    };

class MainMenuScreen : public PushdownState {
    size_t selection = 0;
    bool inMenu;
    TutorialGame* game;
    Controller* controller;
    Texture* fmodLogoTex;
    GameTechRendererInterface* renderer;
public:
    MainMenuScreen(Controller* controller, TutorialGame* game, GameTechRendererInterface* renderer) : controller(controller), game(game), selection(0), inMenu(true), renderer(renderer) {
        fmodLogoTex = renderer->LoadTexture("FMOD Logo Black - White Background.png");

    }

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {


        const std::array<std::string, 5> menuItems = { "Singleplayer", "Host Game", "Join Game", "Credits", "Quit"};

        if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
            selection = std::min(menuItems.size() - 1, selection + 1);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
            if (selection > 0) {
                selection--;
            }
            //selection = std::max(size_t(0), selection - 1);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
            GameMode mode = static_cast<GameMode>(selection);

            switch (mode)
            {
            case GameMode::SINGLEPLAYER:
                *newState = new LobbyScreen(controller, game, true);
                inMenu = false;
                return PushdownResult::Push;
                //game->Start();
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
            if (i == selection) {
                currentItem = "> " + currentItem + " <";
            }
            else {
                currentItem = "  " + currentItem;
            }

            Debug::Print(currentItem, Vector2(0.35f, 0.32f + (0.1f * i)));
        }
        Debug::AddDebugTexture(fmodLogoTex, Vector2(90.0f, 90.0f), Vector2(10.0f, 8.0f));
        return PushdownResult::NoChange;

        //Add FMOD Logo here
        //Debug::AddDebugTexture(fmodLogoTex, Vector2(0.2f, 0.2f), Vector2(10.0f, 10.0f));
    }

    void OnAwake() override {

    }
};

}
