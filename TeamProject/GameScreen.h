#pragma once

#include <PauseScreen.h>

namespace NCL::CSC8503{

class GameScreen : public PushdownState {
public:
    GameScreen(Controller* controller, TutorialGame* game, PlayerController* playerController) : controller(controller), game(game), playerController(playerController) {}
    Controller* controller;
    TutorialGame* game;
    PlayerController* playerController;

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {
        game->UpdateGame(dt);

        if (controller->GetDigital(Controller::DigitalControl::Pause)) {
            *newState = new PauseScreen(controller, game, playerController);
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

}