#include "GameTechRendererInterface.h"

#include <NCLCoreClasses/Frustum.h>
#include <NCLCoreClasses/Window.h>

#include <CSC8503CoreClasses/GameWorld.h>
#include <CSC8503CoreClasses/GameObject.h>
#include <CSC8503CoreClasses/RenderObject.h>

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
	}

}
