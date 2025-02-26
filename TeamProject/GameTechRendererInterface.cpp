#include "GameTechRendererInterface.h"

#include <NCLCoreClasses/Frustum.h>
#include <NCLCoreClasses/Window.h>

#include <CSC8503CoreClasses/GameWorld.h>
#include <CSC8503CoreClasses/GameObject.h>
#include <CSC8503CoreClasses/RenderObject.h>

namespace NCL::CSC8503 {
	void GameTechRendererInterface::collectFrameObjects(GameWorld* world)
	{
		// TODO: Have one window pointer, avoid globals if possible
		auto window = Window::GetWindow();
		auto viewProjMatrix = camera->BuildProjectionMatrix(window->GetScreenAspect()) * camera->BuildViewMatrix();
		auto frustum = Frustum::FromViewProjMatrix(viewProjMatrix);

		frameObjects.clear();

		world->OperateOnContents([&](GameObject* obj) {
			// TODO: Frustum culling, sorting
			if (!obj->IsActive()) return;
			auto render = obj->GetRenderObject();
			if (!render) return;

			float maxScale = std::max({ std::abs(obj->getRenderScale().x), std::abs(obj->getRenderScale().y), std::abs(obj->getRenderScale().z) });
			float bounds = render->GetMesh()->getBoundingRadius() * maxScale;
			if (!frustum.SphereInsideFrustum(obj->GetTransform().getOrigin(), bounds)) return;

			frameObjects.emplace_back(render);
		});
	}
}