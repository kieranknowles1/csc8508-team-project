#pragma once

#include <PauseScreen.h>

using namespace NCL;
using namespace NCL::CSC8503;

namespace NCL::CSC8503 {

    class GameScreen : public PushdownState {
    public:
        GameScreen(Controller* controller, TutorialGame* game) : controller(controller), game(game) {}
        Controller* controller;
        TutorialGame* game;

        PushdownResult OnUpdate(float dt, PushdownState** newState) override {
            game->UpdateGame(dt);

            if (controller->GetDigital(Controller::DigitalControl::Pause)) {
                *newState = new PauseScreen(controller, game);
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