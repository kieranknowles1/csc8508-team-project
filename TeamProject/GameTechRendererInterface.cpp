#include "GameTechRendererInterface.h"

#include <NCLCoreClasses/Frustum.h>
#include <NCLCoreClasses/Window.h>

#include <CSC8503CoreClasses/GameWorld.h>
#include <CSC8503CoreClasses/GameObject.h>
#include <CSC8503CoreClasses/RenderObject.h>
#include <CSC8503CoreClasses/Debug.h>

namespace NCL::CSC8503 {
	GameTechRendererInterface::GameTechRendererInterface(Window* window)
		: window(window)
		, decalSystem(window->GetScreenSize().x, window->GetScreenSize().y)
	{
		initUi();
	}

	void GameTechRendererInterface::collectFrameObjects(GameWorld* world)
	{
		auto viewProjMatrix = camera->BuildProjectionMatrix(window->GetScreenAspect()) * camera->BuildViewMatrix();
		auto frustum = Frustum::FromViewProjMatrix(viewProjMatrix);

		frameObjects.clear();

		world->OperateOnContents([&](GameObject* obj) {
			if (!obj->IsActive()) return;
			auto render = obj->GetRenderObject();
			if (!render) return;

			float maxScale = std::max({ std::abs(obj->getRenderScale().x), std::abs(obj->getRenderScale().y), std::abs(obj->getRenderScale().z) });
			float bounds = render->GetMesh()->getBoundingRadius() * maxScale;
			if (!frustum.SphereInsideFrustum(obj->GetTransform().getOrigin(), bounds)) return;

			frameObjects.emplace_back(render);
		});
	}


    void GameTechRendererInterface::initUi()
    {
        Vector2 screenCenter = Vector2(0.5f, 0.5f);
        Vector4 crosshairColor = Vector4(1, 1, 1, 1); // White crosshair

        float lineLength = 0.02f; // Length of the crosshair lines
        float lineThickness = 0.0025f; // Thickness of each line
        float horizontalLineThickness = 0.0035f;
        float horizontalLineLength = 0.015f;
        float gapSize = 0.0005f; // Gap between the lines

        // Crosshair
        // Left line
        AddUIElement({ Vector2(screenCenter.x - gapSize - lineLength, screenCenter.y),
            Vector2(horizontalLineLength, horizontalLineThickness), crosshairColor });

        // Right line
        AddUIElement({ Vector2(screenCenter.x + gapSize + lineLength, screenCenter.y),
            Vector2(horizontalLineLength, horizontalLineThickness), crosshairColor });

        // Top line
        AddUIElement({ Vector2(screenCenter.x, screenCenter.y + gapSize + lineLength),
            Vector2(lineThickness, lineLength), crosshairColor });

        // Bottom line
        AddUIElement({ Vector2(screenCenter.x, screenCenter.y - gapSize - lineLength),
            Vector2(lineThickness, lineLength), crosshairColor });

		AddPlayerData({ "Player 1", "Red", 4 });
		AddPlayerData({ "Player 2", "Blue", 8 });
		AddPlayerData({ "Player 3", "Green", 2 });
		AddPlayerData({ "Player 4", "Yellow", 11 });

        // Scoreboard
		int players = playerData.size();
        int columns = 3;

        Vector4 scoreboardColor = Vector4(0.1f, 0.1f, 0.1f, 0.7f);
        Vector4 boxColor = Vector4(0.0f, 0.0f, 0.0f, 0.7f);
        Vector4 borderColor = Vector4(0.5f, 0.5f, 0.5f, 0.7f); // Highlight color for borders
        Vector4 textColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f); // Black text color
        Vector2 totalSize = Vector2(0.7f, 0.7f);
        Vector2 boxSize = Vector2(totalSize.x / columns, totalSize.y / 9); // 3 columns, 9 rows (1 title row + 8 data rows)
        float borderThickness = 0.005f; // Thickness of the border

        AddUIElement({ screenCenter, totalSize, scoreboardColor, nullptr, true });

        for (int row = 0; row < 9; ++row) {
            Vector2 rowPosition = Vector2(
                screenCenter.x,
                screenCenter.y - (totalSize.y / 2.0f) + (boxSize.y / 2.0f) + (row * boxSize.y)
            );

            // Add row border
            AddUIElement({ rowPosition, Vector2(totalSize.x, boxSize.y) + Vector2(borderThickness, borderThickness), borderColor, nullptr, true });

            for (int col = 0; col < columns; ++col) {
                Vector2 position = Vector2(
                    screenCenter.x - totalSize.x / 2.0f + boxSize.x / 2.0f + col * boxSize.x,
                    screenCenter.y - totalSize.y / 2.0f + boxSize.y / 2.0f + row * boxSize.y
                );

                // Add main box
                AddUIElement({ position, boxSize, boxColor, nullptr, true });

                // Add text
                Vector2 textPosition = Vector2(
                    position.x - boxSize.x / 2.0f + 0.01f, // Slightly offset to the left
                    position.y + (boxSize.y * 0.25f) // Slightly offset to the bottom
                );

				//title row
                if (row == 0) {
                    std::string text;
                    if (col == 0) {
                        text = "User:";
                    }
                    else if (col == 1) {
                        text = "Colour:";
                    }
                    else {
                        text = "Score:";
                    }
                    AddUITextElement({ textPosition, textColor, text, true });
                }
				// Empty row
                else if (row > players) {
                    AddUITextElement({ textPosition, textColor, "", true });
                }
				// Data rows
                else {
                    std::string text;
                    if (col == 0) {
                        text = playerData[row-1].name;
                    }
                    else if (col == 1) {
                        text = playerData[row-1].color;
                    }
                    else {
                        text = std::to_string(playerData[row-1].score);
                    }
                    AddUITextElement({ textPosition, textColor, text, true });
                }
            }
        }
    }
}
