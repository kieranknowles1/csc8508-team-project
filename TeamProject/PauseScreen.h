#pragma once

#include "AudioEngine.h"
#include "Multiplayer/Server.hpp"

using namespace NCL;
using namespace NCL::CSC8503;

class SingleplayerPauseScreen : public PushdownState {
    int selection = 0;

public:
    SingleplayerPauseScreen(Controller* controller, TutorialGame* game, PlayerController* playerController)
        : controller(controller), game(game), playerController(playerController) {}

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
                if (selection < 1) {
                    selection++;
                    int channelId = audioEngine.PlaySounds("MenuScroll.wav", game->getMainCam()->GetPosition(), -12.0f);
                    audioEngine.SetChannelPitchMultiplier(channelId, 0.9f);
                }
            }
            if (controller->GetDigital(Controller::DigitalControl::MenuUp)) {
                if (selection > 0) {
                    selection--;
                    audioEngine.PlaySounds("MenuScroll.wav", game->getMainCam()->GetPosition(), -12.0f);
                }
            }
            if (controller->GetDigital(Controller::DigitalControl::MenuConfirm)) {
                const std::string pauseSelection = menuItems[selection];

                if (pauseSelection == "Resume") {
                    // Resume all paused channels (they were only paused in OnAwake)
                    if (playerController->getBeamSoundChannel() != -1) {
                        audioEngine.SetChannelPaused(playerController->getBeamSoundChannel(), false);
                    }
                    if (playerController->getHeartbeatChannel() != -1) {
                        audioEngine.SetChannelPaused(playerController->getHeartbeatChannel(), false);
                    }
                    for (int channel : playerController->getJumppadChannels()) {
                        if (channel != -1) {
                            audioEngine.SetChannelPaused(channel, false);
                        }
                    }

                    return PushdownResult::Pop;
                }
                else {
                    // Quit to main menu: Stop and invalidate sound channels
                    if (playerController->getBeamSoundChannel() != -1) {
                        audioEngine.StopChannel(playerController->getBeamSoundChannel());
                        playerController->setBeamSoundChannel(-1);
                    }
                    if (playerController->getHeartbeatChannel() != -1) {
                        audioEngine.StopChannel(playerController->getHeartbeatChannel());
                        playerController->setHeartbeatChannel(-1);
                    }
                    for (int channel : playerController->getJumppadChannels()) {
                        if (channel != -1) {
                            audioEngine.StopChannel(channel);
                        }
                    }
                    playerController->getJumppadChannels().clear();

                    game->StopServer();
                    game->ClearWorld();
                    return PushdownResult::Clear;
                }

                inMenu = false;
            }

            for (int i = 0; i < 2; i++) {
                std::string currentItem = menuItems[i];
                if (i == selection) currentItem = currentItem + " <";
                Debug::Print(currentItem, Vector2(0, 0.50 + (0.10f * i)));
            }
        }

        return PushdownResult::NoChange;
    }

    void OnAwake() override {
        // Pause audio when game is paused
        if (playerController->getBeamSoundChannel() != -1) {
            audioEngine.SetChannelPaused(playerController->getBeamSoundChannel(), true);
            playerController->setBeamSoundPaused(true);
        }
        if (playerController->getHeartbeatChannel() != -1) {
            audioEngine.SetChannelPaused(playerController->getHeartbeatChannel(), true);
        }
        for (int channel : playerController->getJumppadChannels()) {
            if (channel != -1) {
                audioEngine.SetChannelPaused(channel, true);
            }
        }

        // Menu entry sound
        audioEngine.PlaySounds("MenuSelect.wav", game->getMainCam()->GetPosition(), -18.0f);
    }

    void OnSleep() override {
        playerController->setBeamSoundPaused(false);
    }
};
