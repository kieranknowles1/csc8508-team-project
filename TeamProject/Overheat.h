#pragma once
#include "GameTechRendererInterface.h"
#include <array>

#include "btBulletDynamicsCommon.h"

using namespace NCL::Maths;

struct OverheatBar {
    Vector2 position;
    Vector2 size;
    btVector4 colour;
};

enum OverheatState {
    Stopped,
    Shooting,
    Holding,
    Overheating,
    Cooling
};


class Overheat : public UiElement {
public:
    Overheat();
    void render(std::vector<UiSprite>& sprites) override;
    void render(std::vector<UiText>& texts) override {};
    void Animate(float dt);

    void fire();
    void stopFiring();
    bool CanFire() { return (currentState != Overheating && !coolingFromOverheat); }
    float GetOverheatPercentage() const {
        return barSize / maxSize; //Normalize to 0.0 - 1.0
    }

private:

    // Overheating Variables
    float maxFiringTime = 6.0f;
    float overheatTime = 2.5f;
    float holdTime = 0.5f;
    float cooldownTimeMax = 1.0f;

   
    float cooldownTimeCurrent;
    bool canFire = true;
    float maxSize = 0.485f;
    Vector4 startColour = Vector4(0.7f, 0.1f, 0.1f, 1.0f);
    Vector4 endColour = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
    float currentTimer = 0.0f;
    float stoppedAtTime = 0.0f;
    float expansionAtStopped = 0.0f;
    bool coolingFromOverheat = false;
    OverheatBar overheatBar;
    OverheatBar backgroundBar;
    OverheatState currentState = Stopped;
    float barSize = 0.0f;
    Vector4 barColour = Vector4(0.7f, 0.1f, 0.1f, 1.0f);
};