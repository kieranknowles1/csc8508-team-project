#include "Crosshair.h"

Crosshair::Crosshair()
{
    UpdateCrosshairData();
}

void Crosshair::render(std::vector<UiSprite>& sprites) {
    Vector4 crosshairColor = Vector4(1, 1, 1, 1); // White crosshair

    for (const auto& line : lines) {
        sprites.push_back({ line.position, line.size, crosshairColor });
    }

    // Add a small white point at the center of the crosshair
    Vector2 dotSize = Vector2(0.003f, 0.003f);
    sprites.push_back({ screenCenter, dotSize, crosshairColor });
}

// runs once on initial fire
void Crosshair::fire() {
    if (currentState == Idle || currentState == Retracting) {
        currentTimer = currentState == Retracting ? (endTime - currentTimer) : startTime;
        currentState = Expanding;

    }
}

// runs once on stopped
void Crosshair::stopFiring() {
    if (currentState == Expanding) {
        currentTimer = startTime - currentTimer;
    }
    else if (currentState == Firing) {
        currentTimer = endTime;
    }
    currentState = Retracting;
}

// Crosshair basic state machine
void Crosshair::Animate(float dt) {
    switch (currentState) {
    case Expanding:
        currentTimer -= dt;
        if (currentTimer <= 0.0f) {
            currentTimer = 0.0f;
            currentState = Firing;
        }
        expansionFactor = 1.0f + ((1.0f - (currentTimer / startTime)) * (maxExpansion - 1.0f));
        break;

    case Firing:
        expansionFactor = maxExpansion;
        wobbleAmount = sin(timeElapsed * 7.0f) * 0.0075f;
        timeElapsed += dt;
        break;

    case Retracting:
        currentTimer -= dt;
        if (currentTimer <= 0.0f) {
            currentTimer = 0.0f;
            currentState = Idle;
        }
        expansionFactor = 1.0f + ((currentTimer / endTime) * (maxExpansion - 1.0f));
        if (wobbleAmount > 0.01f || wobbleAmount < -0.01f) {
            wobbleAmount = sin(timeElapsed * 7.0f) * 0.0075f;
            timeElapsed += dt; 
        }
        break;

    case Idle:
        timeElapsed = 0.0f;
        expansionFactor = 1.0f;
        break;
    }

    float animatedGap = (gapSize + lineLength) * expansionFactor;
    float amplifiedWobble = wobbleAmount * expansionFactor;
    animatedGap += amplifiedWobble;

    float minimumGap = gapSize + lineLength;
    animatedGap = std::max(animatedGap, minimumGap);

    lines[0].position.x = screenCenter.x - animatedGap;
    lines[1].position.x = screenCenter.x + animatedGap;
    lines[2].position.y = screenCenter.y + animatedGap;
    lines[3].position.y = screenCenter.y - animatedGap;

    if (currentState == Idle) {
        UpdateCrosshairData();
    }
}





void Crosshair::UpdateCrosshairData()
{
    lines[0] = { { screenCenter.x - gapSize - lineLength, screenCenter.y },
                 { horizontalLineLength, horizontalLineThickness } };
    lines[1] = { { screenCenter.x + gapSize + lineLength, screenCenter.y },
                 { horizontalLineLength, horizontalLineThickness } };
    lines[2] = { { screenCenter.x, screenCenter.y + gapSize + lineLength },
                 { lineThickness, lineLength } };
    lines[3] = { { screenCenter.x, screenCenter.y - gapSize - lineLength },
                 { lineThickness, lineLength } };
}
