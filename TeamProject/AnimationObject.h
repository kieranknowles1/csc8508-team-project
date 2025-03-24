#pragma once
#include "Texture.h"
#include "Mesh.h"
#include "Buffer.h"
#include "MeshAnimation.h"
#include "RenderObject.h"

enum AnimationState { // In order of importance - e.g. sliding overrides falling if player is doing both at once
	JUMPING_SPRINT,
	JUMPING_STANDING,
	SLIDING,
	FALLING,
	SPRINTING_FORWARD,
	SPRINTING_BACK,
	SPRINTING_LEFT,
	SPRINTING_RIGHT,
	WALKING_FORWARD,
	WALKING_BACK,
	WALKING_LEFT,
	WALKING_RIGHT,
	IDLE
};
constexpr std::string_view AnimationNames[] = // For printing - e.g. std::cout << "ANIMATED STATE: " << AnimationNames[animationState] << std::endl;
{
	"JUMPING_SPRINT",
	"JUMPING_STANDING",
	"SLIDING",
	"FALLING",
	"SPRINTING_FORWARD",
	"SPRINTING_BACK",
	"SPRINTING_LEFT",
	"SPRINTING_RIGHT",
	"WALKING_FORWARD",
	"WALKING_BACK",
	"WALKING_LEFT",
	"WALKING_RIGHT",
	"IDLE"
};


namespace NCL {
	namespace CSC8503 {
		class AnimationObject {
		public:
			AnimationObject(GameObject* parent);
			~AnimationObject();

			MeshAnimation* getAnimation(AnimationState state);

		private:
			std::unordered_map<AnimationState, MeshAnimation*> animations;
			GameObject* parent;
		};
	}
}