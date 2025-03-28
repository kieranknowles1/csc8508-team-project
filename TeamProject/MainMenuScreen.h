#pragma once

#include "PushdownState.h"
#include <TutorialGame.h>
#include <future>
#include <CSC8503CoreClasses/Debug.h>
#include <iostream>
#include "GameScreen.h"
#include "LobbyScreen.h"
#include "CreditsScreen.h"
#include "GameTechRendererInterface.h"
#include "AudioEngine.h"

namespace NCL::CSC8503 {

    class MainMenuUI : public UiElement {
        
    public:
        MainMenuUI(std::shared_ptr<NCL::Rendering::Texture> logo);

        void render(std::vector<UiSprite>& sprites) override;

        void render(std::vector<UiText>& texts) override;

        void Animate(float dt) override {};

        void InitMenu();

        void UpdateMenu(unsigned int);

    private:

        UIBox background;
		UIBox fmodLogo;
        Text title;
        std::vector<Button> buttons;
        std::vector<Text> buttonTexts;
        Vector4 activeButton = Vector4(0.3, 0.3, 0.3, 0.5);
        Vector4 inactiveButton = Vector4(0, 0, 0, 1);
        std::shared_ptr<NCL::Rendering::Texture> fmodLogoTex;
    };
    
    class MainMenuScreen : public PushdownState {
        size_t selection = 0;
        TutorialGame* game;
        Controller* controller;
        std::shared_ptr<NCL::Rendering::Texture> fmodLogoTex;
        GameTechRendererInterface* renderer;
        ResourceManager* resourceManager;
		std::unique_ptr<MainMenuUI> ui;
        float connectionFailedTime = 0;

    public:
        MainMenuScreen(Controller* controller, TutorialGame* game, GameTechRendererInterface* renderer) : controller(controller), game(game), selection(0), renderer(renderer) {

            resourceManager = game->GetResourceManager();
            fmodLogoTex = resourceManager->getTextures().get("FMOD Logo White - Black Background1.png");
			
            ui = std::make_unique<MainMenuUI>(fmodLogoTex);
			renderer->AddUiElement(ui.get());
			ui->SetActive(true);
			ui->UpdateMenu((int)selection);
        }

        PushdownResult OnUpdate(float dt, PushdownState** newState) override {
            bool ok = true;

            if (connectionFailedTime > 0) {
                connectionFailedTime -= dt;
                Debug::Print("- Connection failed", Vector2(0.05f, 0.95f));
            }

            if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
                if (selection < 4) {
                    selection++;
					UpdateSelection((int)selection);
                    int channelId = audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), -12.0f);
                    audioEngine.SetChannelPitchMultiplier(channelId, 0.9f);
                }
            }
            if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
                if (selection > 0) {
                    selection--;
					UpdateSelection((int)selection);
                    audioEngine.PlaySounds("MenuScroll.wav", Vector3(0, 0, 0), -12.0f);
                }
            }
            if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
                GameMode mode = static_cast<GameMode>(selection);

                game->SetGameMode(mode);
                switch (mode)
                {
                case GameMode::SINGLEPLAYER:
                    game->SetGameMode(GameMode::SINGLEPLAYER);
                    audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
                    game->Start();
                    *newState = new GameScreen(controller, game, game->GetPlayerController());
                    ui->SetActive(false);
                    return PushdownResult::Push;

                case GameMode::HOST_GAME:
                    audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);

                    ok = game->StartMultiplayerGame(true);
                    if (ok) {
                        ui->SetActive(false);
                        *newState = new HostLobbyScreen(controller, game);
                        return PushdownResult::Push;
                    }
                    connectionFailedTime = 3.0f;
                    return PushdownResult::NoChange;

                case GameMode::JOIN_GAME:
                    ok = game->StartMultiplayerGame(false);
                    audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);

                    if (ok) {
						ui->SetActive(false);
                        *newState = new ClientLobbyScreen(controller, game);
                        return PushdownResult::Push;
                    }
                    connectionFailedTime = 3.0f;
                    return PushdownResult::NoChange;

                case GameMode::CREDITS:
					ui->SetActive(false);
                    game->SetGameMode(GameMode::CREDITS);
                    audioEngine.PlaySounds("MenuSelect.wav", Vector3(0, 0, 0), -18.0f);
                    *newState = new CreditsScreen(controller, Assets::CREDITS);
                    return PushdownResult::Push;

                case GameMode::QUIT:
                    renderer->ClearUIElemets();
                    game->SetGameMode(GameMode::QUIT);
                    return PushdownResult::Pop;

                default: assert(false);
                }

                return PushdownResult::Push;
            }
            return PushdownResult::NoChange;

        }

        void OnAwake() override {

            game->getMainCam()->SetPosition({ 0, 0, 0 });
            audioEngine.Update(game->getMainCam());
            renderer->AddUiElement(ui.get());
			ui->SetActive(true);
        }

		void UpdateSelection(unsigned int selection) {
			ui->UpdateMenu(selection);
		}
    };
}
