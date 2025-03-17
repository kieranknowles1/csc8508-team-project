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

enum CrosshairState {
    Idle,
    Expanding,
    Firing,
    Retracting
};


class Crosshair : public UiElement {
public:
    Crosshair();
    void render(std::vector<UiSprite>& sprites) override;
    void render(std::vector<UiText>& texts) override {};
    void Animate(float dt);

    void fire();
    void stopFiring();

private:
    void UpdateCrosshairData();

    Vector2 screenCenter = Vector2(0.5f, 0.5f);
    float lineLength = 0.02f;
    float lineThickness = 0.0025f;
    float horizontalLineThickness = 0.0035f;
    float horizontalLineLength = 0.015f;
    float gapSize = 0.018f;

    std::array<CrosshairLine, 4> lines;
    float startTime = 1.0f;
    float endTime = 1.0f;
    float timeElapsed = 0.0f;
    float maxExpansion = 4.0f;
    CrosshairState currentState = Idle;
    float currentTimer = 0.0f;
    bool reversing = false;
    float wobbleAmount = 0.0f;
    float expansionFactor = 1.0f;
};