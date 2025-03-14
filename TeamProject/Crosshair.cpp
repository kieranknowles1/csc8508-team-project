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

void Crosshair::fire() { fired = false; }
void Crosshair::Animate(float dt)
{
 
    if (!fired) {
        pulse += dt*2.5f;
    }
    if (pulse > 0.2f) {
        pulse = 0.0f;
        fired = true;
    }



    // Animate the crosshair by setting each line's length to pulse
    // This will make the crosshair lines grow and shrink
    lines[0].position.x = screenCenter.x + gapSize + (pulse * 0.02f);
    lines[1].position.x = screenCenter.x - gapSize - (pulse * 0.02f);
    lines[2].position.y = screenCenter.y - gapSize - (pulse * 0.02f);
    lines[3].position.y = screenCenter.y + gapSize + (pulse * 0.02f);
}

void Crosshair::UpdateCrosshairData()
{
    screenCenter = Vector2(0.5f, 0.5f);
    lineLength = 0.01f;
    lineThickness = 0.0025f;
    horizontalLineThickness = 0.0035f;
    horizontalLineLength = 0.006f;
    gapSize = 0.013f;

    lines[0] = { { screenCenter.x - gapSize - horizontalLineLength* 0.5f, screenCenter.y },
                 { horizontalLineLength, horizontalLineThickness } };

    lines[1] = { { screenCenter.x + gapSize + horizontalLineLength * 0.5f, screenCenter.y },
                 { horizontalLineLength, horizontalLineThickness } };

    lines[2] = { { screenCenter.x, screenCenter.y + gapSize + lineLength },
                 { lineThickness, lineLength } };

    lines[3] = { { screenCenter.x, screenCenter.y - gapSize - lineLength },
                 { lineThickness, lineLength } };
}
