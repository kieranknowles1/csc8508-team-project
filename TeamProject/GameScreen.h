#pragma once

#include <PauseScreen.h>
#include "PushdownState.h"
#include "EndScreenMP.h"
#include "EndScreenSP.h"


namespace NCL::CSC8503 {

    class GameScreen : public PushdownState {
    public:
        GameScreen(Controller* controller, TutorialGame* game, PlayerController* playerController) : controller(controller), game(game), playerController(playerController) {}
        Controller* controller;
        TutorialGame* game;
        PlayerController* playerController;

        PushdownResult OnUpdate(float dt, PushdownState** newState) override {
            game->UpdateGame(dt);

            if (game->getSPEnd()) {
                *newState = new EndScreenSP(controller, game, game->GetSPMode()->getScore());
                std::cout << "Singeplayer game ended" << std::endl;
                return PushdownResult::Push;
            }
            if (controller->GetDigital(Controller::DigitalControl::Pause)) {
                *newState = new SingleplayerPauseScreen(controller, game, playerController);
                std::cout << "Game entered pause state \n";
                return PushdownResult::Push;
            }
            if (controller->GetDigital(Controller::DigitalControl::PauseQuit)) {
                Debug::Print("Going back to main menu", Vector2(0.1f, 0.3f), Vector4(0, 0, 0, 1));
                return PushdownResult::Pop;
            }
            return PushdownResult::NoChange;
        };
        void OnAwake() override {
            std::cout << "Game state active\n";
        }
    };


    class MultiplayerGameScreen : public PushdownState {
    public:
        MultiplayerGameScreen(Controller* controller, TutorialGame* game, PlayerController* playerController)
            : controller(controller), game(game), playerController(playerController), isPaused(false), selection(0) {
        }

        PushdownResult OnUpdate(float dt, PushdownState** newState) override {
            if (controller->GetDigital(Controller::DigitalControl::Pause)) {
                isPaused = true;
                game->SetPlayerUpdatePaused(true);
            }
            gameTimer += dt;
            if ((maxGameTime - gameTimer) > 0.1f) {
                std::string asString = std::to_string((int)((maxGameTime - gameTimer) * 100));
                asString = asString.substr(0, asString.size() - 2) + "." + asString.substr(asString.size() - 2, 2);
                Debug::Print("TIME: " + asString, Vector2(0.75, 0.1));
            }

            Vector3 camFwd = game->getMainCam()->GetForwardVector();
            std::string camLook = "Camera Forward: (" + std::to_string(camFwd.x) + ", " + std::to_string(camFwd.y) + ", " + std::to_string(camFwd.z) + ")";
            Debug::Print(camLook, Vector2(0.01f, 0.5f));

           
            if (gameTimer >= maxGameTime) {
                *newState = new EndScreenMP(controller, game);
                return PushdownResult::Push;
            }
            if (!isPaused) {
                game->UpdatePlayer(dt);
            }
            else {
                game->UpdatePlayer(dt, true);
            }

            game->UpdateGame(dt); // Always update the game loop

            if (isPaused) {
                int beamChannel = playerController->getBeamSoundChannel();
                if (beamChannel != -1) {
                    audioEngine.SetChannelPaused(beamChannel, true);
                    playerController->setBeamSoundPaused(true);
                }
                return HandlePauseMenu(newState);

            }

            return PushdownResult::NoChange;
        }

        void OnAwake() override {
            std::cout << "Multiplayer game state active\n";
            isPaused = false;
            game->SetPlayerUpdatePaused(false);
        }

    private:
        bool isPaused = false;
        size_t selection = 0;
        Controller* controller;
        TutorialGame* game;
        PlayerController* playerController;
        float gameTimer = 0;
        float maxGameTime = 300.0f;

        PushdownResult HandlePauseMenu(PushdownState** newState) {
            const std::array<std::string, 2> menuItems = { "Resume", "Exit" };

            if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
                if (selection < menuItems.size() - 1) {
                    selection++;
                    audioEngine.PlaySounds("MenuScroll.wav", game->getMainCam()->GetPosition(), -12.0f);
                }
            }
            if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
                if (selection > 0) {
                    selection--;
                    audioEngine.PlaySounds("MenuScroll.wav", game->getMainCam()->GetPosition(), -12.0f);
                }
            }
            if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
                audioEngine.PlaySounds("MenuSelect.wav", game->getMainCam()->GetPosition(), -18.0f);

                if (selection == 0) { // Resume
                    isPaused = false;
                    game->SetPlayerUpdatePaused(false);
                    if (playerController->getBeamSoundChannel() != -1 && playerController->getController()->GetDigital(Controller::DigitalControl::Fire)) {
                        audioEngine.SetChannelPaused(playerController->getBeamSoundChannel(), false);
                        playerController->setBeamSoundPaused(false);
                    }
                }
                else if (selection == 1) { // Exit
                    game->ClearWorld();
                    return PushdownResult::Clear;
                }
            }

            for (int i = 0; i < menuItems.size(); ++i) {
                std::string label = (i == selection) ? "> " + menuItems[i] + " <" : "  " + menuItems[i];
                Debug::Print(label, Vector2(0.4f, 0.45f + i * 0.1f));
            }

            return PushdownResult::NoChange;
        }
    };
}

