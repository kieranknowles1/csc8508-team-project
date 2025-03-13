#pragma once

#include <memory>

#include "Controller.h"

namespace NCL {
    // Combines two controllers
    class HybridController : public Controller {
    public:
        HybridController(std::unique_ptr<Controller> primary, std::unique_ptr<Controller> secondary)
            : primary(std::move(primary))
            , secondary(std::move(secondary))
        {}

        float GetAnalogue(AnalogueControl axis) const override {
            return primary->GetAnalogue(axis)
                + secondary->GetAnalogue(axis);
        }
        bool GetDigital(DigitalControl control) const override {
            return primary->GetDigital(control) ||
                secondary->GetDigital(control);
        }
        void Update(float dt) override {
            primary->Update(dt);
            secondary->Update(dt);
        }
    protected:
        std::unique_ptr<Controller> primary;
        std::unique_ptr<Controller> secondary;
    };
}
