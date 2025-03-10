#pragma once

#include "DecalSystem.h"
#include <string>
#include <vector>
#include <memory>
#include "PointLight.h"

#include <NCLCoreClasses/Vector.h>

namespace NCL::Rendering {
	class Mesh;
	class Texture;
	class Shader;
}

namespace NCL {
	class Camera;
	class Window;
}

namespace NCL::CSC8503 {
	class GameWorld;
	class RenderObject;

	struct UiSprite {
		// TODO: Need a UV field
		Maths::Vector2 position;
		Maths::Vector2 size;
		Maths::Vector4 color;
		std::shared_ptr<Rendering::Texture> texture;
	};

    class UiElement {
        bool enabled = true;

    public:
        virtual void render(std::vector<UiSprite>& sprites) = 0;
    };

	class GameTechRendererInterface
	{
	public:
		virtual void drawFrame(float dt) = 0;

		GameTechRendererInterface(Window* window);
		virtual ~GameTechRendererInterface() = default;

		virtual Rendering::Mesh* LoadMesh(const std::string& name) = 0;
		virtual Rendering::Texture*	LoadTexture(const std::string& name) = 0;

		bool GetHDROn() const {
			return hdrOn;
		}

		void SetHDROn(bool toggle) {
			hdrOn = toggle;
		}

		DecalSystem& GetDecalSystem() {
			return decalSystem;
		}
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

        void AddUiElement(UiElement* elem) {
            uiElements.push_back(elem);
        }

	protected:
		//adding bools to toggle post processing. Must be accessible from the specific renderer
		bool hdrOn = true;
		bool vignetteOn = false;
		float vignettePulse = 0;
		Window* window;
		Camera* camera = nullptr;
		std::vector<UiElement*> uiElements;
		std::vector<RenderObject*> frameObjects;
        std::vector<UiSprite> frameSprites;
		std::vector<PointLight*> lights;
		DecalSystem decalSystem;
	};
}

