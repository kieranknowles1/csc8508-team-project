#pragma once

#include "DecalSystem.h"
#include <string>
#include <vector>

namespace NCL::Rendering {
	class Mesh;
	class Texture;
	class Shader;
}

namespace NCL {
	class Camera;
}

namespace NCL::CSC8503 {
	class GameWorld;
	class RenderObject;

	class GameTechRendererInterface
	{
	public:
		virtual Rendering::Mesh* LoadMesh(const std::string& name) = 0;
		virtual Rendering::Texture*	LoadTexture(const std::string& name) = 0;

		bool GetHDROn() const {
			return hdrOn;
		}

		void SetHDROn(bool toggle) {
			hdrOn = toggle;
		}

		virtual DecalSystem& GetDecalSystem() = 0;
		void setCamera(Camera* cam) {
			camera = cam;
		}

		// Collect a list of RenderObjects that need to be rendered this frame
		void collectFrameObjects(GameWorld* world);

		bool GetVignetteOn() const {
			return vignetteOn;
		}

		void SetVignetteOn(bool toggle) {
			vignetteOn = toggle;
		}

		void SetVignettePulse(float dt) {
			vignettePulse = dt;
		}

	protected:
		//adding bools to toggle post processing. Must be accessible from the specific renderer
		bool hdrOn = true;
		bool vignetteOn = false;
		float vignettePulse = 0;
		Camera* camera = nullptr;
		std::vector<RenderObject*> frameObjects;
	};
}

