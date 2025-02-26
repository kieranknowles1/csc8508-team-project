#include "GameTechRendererInterface.h"

#include <CSC8503CoreClasses/GameWorld.h>
#include <CSC8503CoreClasses/GameObject.h>

namespace NCL::CSC8503 {
	void GameTechRendererInterface::collectFrameObjects(GameWorld* world)
	{
		frameObjects.clear();

		world->OperateOnContents([&](GameObject* obj) {
			// TODO: Frustum culling, sorting
			if (obj->IsActive() && obj->GetRenderObject()) {
				frameObjects.emplace_back(obj->GetRenderObject());
			}
		});
	}
}