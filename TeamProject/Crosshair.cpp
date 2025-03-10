// Crosshair.cpp
#include "Crosshair.h"
using namespace NCL::Maths;

void Crosshair::render(std::vector<UiSprite>& sprites) {
    Vector2 screenCenter = Vector2(0.5f, 0.5f);
    Vector4 crosshairColor = Vector4(1, 1, 1, 1); // White crosshair

    float lineLength = 0.02f; // Length of the crosshair lines
    float lineThickness = 0.0025f; // Thickness of each line
    float horizontalLineThickness = 0.0035f;
    float horizontalLineLength = 0.015f;
    float gapSize = 0.0005f; // Gap between the lines

    // Draw the crosshair lines
    // Left line
    sprites.push_back({ Vector2(screenCenter.x - gapSize - lineLength, screenCenter.y),
                        Vector2(horizontalLineLength, horizontalLineThickness), crosshairColor });

    // Right line
    sprites.push_back({ Vector2(screenCenter.x + gapSize + lineLength, screenCenter.y),
                        Vector2(horizontalLineLength, horizontalLineThickness), crosshairColor });

    // Top line
    sprites.push_back({ Vector2(screenCenter.x, screenCenter.y + gapSize + lineLength),
                        Vector2(lineThickness, lineLength), crosshairColor });

    // Bottom line
    sprites.push_back({ Vector2(screenCenter.x, screenCenter.y - gapSize - lineLength),
                        Vector2(lineThickness, lineLength), crosshairColor });
}
