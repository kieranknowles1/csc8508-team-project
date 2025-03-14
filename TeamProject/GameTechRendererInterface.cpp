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
		initLasers();
	}

	void GameTechRendererInterface::collectFrameObjects(GameWorld* world)
	{
		auto viewProjMatrix = camera->BuildProjectionMatrix(window->GetScreenAspect()) * camera->BuildViewMatrix();
		auto frustum = Frustum::FromViewProjMatrix(viewProjMatrix);

		frameObjects.clear();
		lights = world->GetLights();
		world->OperateOnContents([&](GameObject* obj) {
			if (!obj->IsActive()) return;
			auto render = obj->GetRenderObject();
			if (!render) return;

			float maxScale = std::max({ std::abs(obj->getRenderScale().x), std::abs(obj->getRenderScale().y), std::abs(obj->getRenderScale().z) });
			float bounds = render->GetMesh()->getBoundingRadius() * maxScale;
			if (!frustum.SphereInsideFrustum(obj->GetTransform().getOrigin(), bounds)) return;

			frameObjects.emplace_back(render);
			PointLight* objLight = obj->getLight();
			if (objLight != nullptr) {
				objLight->worldPosition = obj->GetTransform().getOrigin();
				lights.push_back(objLight);
			}
		});

        frameSprites.clear();
        for (auto elem : uiElements) {
            if (elem->IsActive()) {
                elem->render(frameSprites);
            }
        }
		lights = world->GetLights();
	}
}
