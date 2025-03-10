#pragma once
#include "GameTechRendererInterface.h"

class Crosshair : public UiElement {
public:
    void render(std::vector<UiSprite>& sprites) override;
};