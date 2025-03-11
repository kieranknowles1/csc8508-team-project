#pragma once

using namespace NCL;
using namespace NCL::CSC8503;

class PauseScreen : public PushdownState {
    int selection = 0;

public:
    PauseScreen(Controller* controller, TutorialGame* game) : controller(controller), game(game) {}
    Controller* controller;
    TutorialGame* game;

    PushdownResult OnUpdate(float dt, PushdownState** newState) override {
        if (controller->GetDigital(Controller::DigitalControl::Unpause)) {
            return PushdownResult::Pop;
        }
        Debug::Print("Press P to unpause the game!", Vector2(20, 20), Vector4(0, 0, 0, 1));
        //return PushdownResult::NoChange;
        //Debug::Print("Press U to unpause the game!", Vector2(10, 10), Vector4(0, 0, 0, 1));

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
                Debug::Print(currentItem, Vector2(1, 50 + (10 * i)));
            }
        }

        return PushdownResult::NoChange;
    }
    void OnAwake() override {
    }
};