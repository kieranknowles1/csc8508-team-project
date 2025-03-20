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
#include "TextureUiElement.h"
#include "AudioEngine.h"

namespace NCL::CSC8503 {

    /*enum class GameMode {
        SINGLEPLAYER,
        HOST_GAME,
        JOIN_GAME,
        CREDITS,
        QUIT,
    };*/

class MainMenuScreen : public PushdownState {
    size_t selection = 0;
    TutorialGame* game;
    Controller* controller;
    std::shared_ptr<NCL::Rendering::Texture> fmodLogoTex;
    GameTechRendererInterface* renderer;
    ResourceManager* resourceManager;
    std::unique_ptr<TextureUiElement> textureUiElement;

public:
    MainMenuScreen(Controller* controller, TutorialGame* game, GameTechRendererInterface* renderer) : controller(controller), game(game), selection(0), renderer(renderer) {
    
        resourceManager = game->GetResourceManager();
        fmodLogoTex = resourceManager->getTextures().get("FMOD Logo White - Black Background1.png");
        UiSprite fmodLogo = { Vector2(0.9f, 0.1f), Vector2(0.2f, 0.2f), Vector4(1,1,1,1), fmodLogoTex };
        textureUiElement = std::make_unique<TextureUiElement>(fmodLogo);
        renderer->AddUiElement(textureUiElement.get());
        textureUiElement->SetActive(true);
    }

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {


        const std::array<std::string, 5> menuItems = { "Singleplayer", "Host Game", "Join Game", "Credits", "Quit"};

        if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
            selection = std::min(menuItems.size() - 1, selection + 1);
            audioEngine.PlaySounds("MenuScroll.wav", Vector3(0,0,0), 0.1f);
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
            if (selection > 0) {
                selection--;
                audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), 0.1f);
            }
        }
        if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
            GameMode mode = static_cast<GameMode>(selection);

            switch (mode)
            {
            case GameMode::SINGLEPLAYER:
                textureUiElement->SetActive(false);
                game->SetGameMode(GameMode::SINGLEPLAYER);
                game->Start();
                *newState = new GameScreen(controller, game);
                break;
            case GameMode::HOST_GAME:
                textureUiElement->SetActive(false);
                *newState = new HostLobbyScreen(controller, game);
                return PushdownResult::Push;
                //game->StartMultiplayerGame(true);
                //*newState = new HostLobbyScreen(controller, game);
                //return PushdownResult::Push
                //game->SetGameMode(GameMode::HOST_GAME);
                //game->JoinGame(true); //This is host game
                break;
            case GameMode::JOIN_GAME:
                textureUiElement->SetActive(false);
                *newState = new ClientLobbyScreen(controller, game);
                return PushdownResult::Push;
                //game->SetGameMode(GameMode::JOIN_GAME);
                //game->JoinGame(false); //This is join game
                break;
            case GameMode::CREDITS:
                textureUiElement->SetActive(false);
                game->SetGameMode(GameMode::CREDITS);
                *newState = new CreditsScreen(controller, Assets::CREDITS);
                return PushdownResult::Push;
                break;
            case GameMode::QUIT:
                renderer->ClearUIElemets();
                game->SetGameMode(GameMode::QUIT);
                return PushdownResult::Pop;
            default: assert(false);
            }

            *newState = new GameScreen(controller, game);
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
        return PushdownResult::NoChange;

        //Add FMOD Logo here as well as the line "Audio Engine: FMOD Studio by Firelight Technologies Pty Ltd."
    }

    void OnAwake() override {
        if (!textureUiElement) {  // Check if the element exists
            fmodLogoTex = resourceManager->getTextures().get("FMOD Logo Black - White Background1.png");
            UiSprite fmodLogo = { Vector2(0.8f, 0.5f), Vector2(0.2f, 0.2f), Vector4(1,1,1,1), fmodLogoTex };
            textureUiElement = std::make_unique<TextureUiElement>(fmodLogo);
        }
        renderer->AddUiElement(textureUiElement.get());
        textureUiElement->SetActive(true);
    }
};

}
