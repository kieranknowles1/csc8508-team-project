#include "Overheat.h"



Overheat::Overheat() {
    overheatBar = { Vector2(0.5,0.2),Vector2(0.0,0.06),startColour };
    backgroundBar = { Vector2(0.5,0.2),Vector2(0.5,0.075),Vector4(0.2,0.2,0.2,0.4f) };
}


void Overheat::render(std::vector<UiSprite>& sprites) {

    sprites.push_back({ backgroundBar.position, backgroundBar.size, backgroundBar.colour });
    sprites.push_back({ overheatBar.position, overheatBar.size, overheatBar.colour });
 
}

// runs once on initial fire
void Overheat::fire() {
    if (currentState == Stopped) {
        currentTimer = maxFiringTime;
        currentState = Shooting;
    }
    else if(currentState == Holding){
        currentTimer = stoppedAtTime;
        currentState = Shooting;
    }
    else if (currentState == Cooling) {
        currentTimer = maxFiringTime * (1-(overheatBar.size.x / maxSize));
        currentState = Shooting;
    }
}

// runs once on stopped
void Overheat::stopFiring() {
    if (currentState == Shooting) {
        currentTimer = holdTime;
        currentState = Holding;
    }
}

// Overheating basic state machine
void Overheat::Animate(float dt) {
    switch (currentState) {
    case Shooting:
        currentTimer -= dt;
        stoppedAtTime = currentTimer;
        expansionAtStopped = (1 - (currentTimer / maxFiringTime)) * maxSize;
        if (currentTimer <= 0.0f) {
            currentTimer = overheatTime;
            currentState = Overheating;
        }
        barSize = expansionAtStopped;
        if (currentTimer <= (maxFiringTime / 4)) {
            barColour = Vector4(0.5f, 0.05f, 0.05f, 1.0f);
        }
        else {
            barColour = Vector4(0.7f, 0.1f, 0.1f, 0.4f);
        }
   
        break;
    case Holding:
        currentTimer -= dt;
        if (currentTimer <= 0.0f) {
            cooldownTimeCurrent = cooldownTimeMax * (overheatTime/stoppedAtTime);
            currentTimer = cooldownTimeCurrent;
            currentState = Cooling;
        }
        barColour = Vector4(0.9f, 0.3f, 0.3f, 0.4f);
        break;
    case Overheating:
        currentTimer -= dt;
        if (currentTimer <= 0.0f) {
            cooldownTimeCurrent = cooldownTimeMax;
            currentTimer = cooldownTimeCurrent;
            currentState = Cooling;
            coolingFromOverheat = true;
        }
        barColour = Vector4(0.2f, 0.0f, 0.0f, 1.0f);
        break;
    case Cooling:
        currentTimer -= dt;
        if (currentTimer <= 0.0f) {
            currentTimer = 0.0f;
            currentState = Stopped;
            coolingFromOverheat = false;
        }
        barSize = (currentTimer / cooldownTimeCurrent) * (expansionAtStopped);
        barColour = Vector4(0.4f, 0.7f, 1.0f, 0.4f);
        break;
    case Stopped:
        break;
    }
    overheatBar.size.x = barSize;
    overheatBar.colour = barColour;
}
