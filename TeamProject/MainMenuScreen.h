#pragma once

//#include "PushdownMachine.h"
#include "PushdownState.h"
#include <TutorialGame.h>
#include <future>
//#include "Window.h"
#include <CSC8503CoreClasses/Debug.h>
#include <iostream>
#include "GameScreen.h"
#include "LobbyScreen.h"
#include "CreditsScreen.h"
#include "GameTechRendererInterface.h"
#include "LobbyScreen.h"
#include "TextureUiElement.h"
#include "AudioEngine.h"

namespace NCL::CSC8503 {

    class MainMenuScreen : public PushdownState {
        size_t selection = 0;
        TutorialGame* game;
        Controller* controller;
        std::shared_ptr<NCL::Rendering::Texture> fmodLogoTex;
        GameTechRendererInterface* renderer;
        ResourceManager* resourceManager;
        std::unique_ptr<TextureUiElement> textureUiElement;
        float connectionFailedTime = 0;

    public:
        MainMenuScreen(Controller* controller, TutorialGame* game, GameTechRendererInterface* renderer) : controller(controller), game(game), selection(0), renderer(renderer) {

            resourceManager = game->GetResourceManager();
            fmodLogoTex = resourceManager->getTextures().get("FMOD Logo White - Black Background1.png");
            UiSprite fmodLogo = { Vector2(0.95f, 0.05f), Vector2(0.1f, 0.1f), Vector4(1,1,1,1), fmodLogoTex };
            textureUiElement = std::make_unique<TextureUiElement>(fmodLogo);
            renderer->AddUiElement(textureUiElement.get());
            textureUiElement->SetActive(true);
        }

        PushdownResult OnUpdate(float dt, PushdownState** newState) override {
            const std::array<std::string, 5> menuItems = { "Singleplayer", "Host Game", "Join Game", "Credits", "Quit" };
            bool ok = true;

            if (connectionFailedTime > 0) {
                connectionFailedTime -= dt;
                Debug::Print("Connection failed", Vector2(0.05f, 0.9f));
            }

            if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
                if (selection < menuItems.size() - 1) {  // Only increase if not at the last option
                    selection++;
                    int channelId = audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), -12.0f);
                    audioEngine.SetChannelPitchMultiplier(channelId, 0.9f);
                }
            }
            if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
                if (selection > 0) {
                    selection--;
                    audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), -12.0f);
                }
            }
            if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
                GameMode mode = static_cast<GameMode>(selection);

                game->SetGameMode(mode);
                switch (mode)
                {
                case GameMode::SINGLEPLAYER:
                    textureUiElement->SetActive(false);
                    game->SetGameMode(GameMode::SINGLEPLAYER);
                    audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
                    game->Start();
                    *newState = new GameScreen(controller, game, game->GetPlayerController());
                    break;
                case GameMode::HOST_GAME:
                    textureUiElement->SetActive(false);
                    audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
                    game->StartMultiplayerGame(true);
                    *newState = new HostLobbyScreen(controller, game);
                    return PushdownResult::Push;
                    //game->StartMultiplayerGame(true);
                    //*newState = new HostLobbyScreen(controller, game);
                    //return PushdownResult::Push
                    //game->SetGameMode(GameMode::HOST_GAME);
                    //game->JoinGame(true); //This is host game
                    break;
                case GameMode::JOIN_GAME:
                    ok = game->StartMultiplayerGame(false);
                    audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);

                    if (ok) {
                        textureUiElement->SetActive(false);
                        *newState = new ClientLobbyScreen(controller, game);
                        return PushdownResult::Push;
                    }
                    connectionFailedTime = 3.0f;
                    return PushdownResult::NoChange;
                    break;
                case GameMode::CREDITS:
                    textureUiElement->SetActive(false);
                    game->SetGameMode(GameMode::CREDITS);
                    audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
                    *newState = new CreditsScreen(controller, Assets::CREDITS);
                    return PushdownResult::Push;
                    break;
                case GameMode::QUIT:
                    // audioEngine.Shutdown();
                    renderer->ClearUIElemets();
                    game->SetGameMode(GameMode::QUIT);
                    return PushdownResult::Pop;
                default: assert(false);
                }

                return PushdownResult::Push;
            }

            //Render menu
            for (int i = 0; i < menuItems.size(); i++) {
                std::string currentItem = menuItems[i];
                if (i == (int)selection) {
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

            //The below while function would fix the sound abruptly cutting off when ex
            //exiting from the lobby to the main menu by adding a delay, not ideal. 

            // Wait until no sounds are playing before restarting the audio engine
            //while (audioEngine.IsAnySoundPlaying()) {
            //    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small delay to check again
            //}

            // Now restart the audio engine
            //audioEngine.Shutdown();
            //audioEngine.Init();
            game->getMainCam()->SetPosition({ 0, 0, 0 });
            audioEngine.Update(game->getMainCam());

            if (!textureUiElement) {  // Check if the element exists
                fmodLogoTex = resourceManager->getTextures().get("FMOD Logo Black - White Background1.png");
                UiSprite fmodLogo = { Vector2(0.95f, 0.05f), Vector2(0.1f, 0.1f), Vector4(1,1,1,1), fmodLogoTex };
                textureUiElement = std::make_unique<TextureUiElement>(fmodLogo);
            }
            renderer->AddUiElement(textureUiElement.get());
            textureUiElement->SetActive(true);

            //audioEngine.Update(game->getMainCam());


            //audioEngine.Shutdown();
            //audioEngine.Init();
        }
    };
}
