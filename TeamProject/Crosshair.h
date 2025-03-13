#pragma once
#include "GameTechRendererInterface.h"
#include <array>

#include "btBulletDynamicsCommon.h"

using namespace NCL::Maths;

struct CrosshairLine {
    // Use NCL::Maths if this doesn't work
    Vector2 position;
    Vector2 size;
};

class Crosshair : public UiElement {
public:
    Crosshair();
    void render(std::vector<UiSprite>& sprites) override;
    void render(std::vector<UiText>& texts) override {};
    void Animate(float dt);

    void fire();

private:
    void UpdateCrosshairData();

    Vector2 screenCenter = Vector2(0.5f, 0.5f);
    float lineLength = 0.02f;
    float lineThickness = 0.0025f;
    float horizontalLineThickness = 0.0035f;
    float horizontalLineLength = 0.015f;
    float gapSize = 0.0005f;

    // Pulse animation
    float pulse = 0.0f;
    std::array<CrosshairLine, 4> lines;
    bool fired = false;
};