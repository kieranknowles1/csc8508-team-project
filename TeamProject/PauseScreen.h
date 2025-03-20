#pragma once

#include "AudioEngine.h"

using namespace NCL;
using namespace NCL::CSC8503;

class PauseScreen : public PushdownState {
    int selection = 0;

public:
    PauseScreen(Controller* controller, TutorialGame* game, PlayerController* playerController) : controller(controller), game(game), playerController(playerController) {}
    Controller* controller;
    TutorialGame* game;
    PlayerController* playerController;

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {
        if (controller->GetDigital(Controller::DigitalControl::Unpause)) {
            return PushdownResult::Pop;
        }

        const std::string resumeGame = "Resume";
        const std::string exitGame = "Exit";

        bool inMenu = true;

        std::string menuItems[2] = { resumeGame, exitGame };

        if (inMenu) {
            if (controller->GetDigital(Controller::DigitalControl::MenuDown)) {
                selection = std::min(1, selection + 1);
            }
            if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
                selection = std::max(0, selection - 1);
            }
            if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
                const std::string pauseSelection = menuItems[selection];

                if (pauseSelection == "Resume") {
                    return PushdownResult::Pop;
                }
                else {
                    game->ClearWorld();
                    return PushdownResult::Clear;
                }
                inMenu = false;
            }

            for (int i = 0; i < 2; i++) {
                std::string currentItem = menuItems[i];
                if (i == selection) currentItem = currentItem + " <";
                Debug::Print(currentItem, Vector2(0, 0.50 + (0.10 * i)));
            }
        }

        return PushdownResult::NoChange;
    }
    void OnAwake() override {
        if (playerController->getBeamSoundChannel() != -1) {
            audioEngine.SetChannelPaused(playerController->getBeamSoundChannel(), true);
            playerController->setBeamSoundPaused(true);
        }
    }

    void OnSleep() override {
        if (playerController->getBeamSoundChannel() != -1) {
            if (playerController->getController()->GetDigital(Controller::DigitalControl::Fire)) {
                audioEngine.SetChannelPaused(playerController->getBeamSoundChannel(), false);
                playerController->setBeamSoundPaused(false);
            }
            else {
                audioEngine.SetChannelVolume(playerController->getBeamSoundChannel(), -100.0f);
                playerController->setBeamSoundChannel(-1);
            }
        }
    }
};
