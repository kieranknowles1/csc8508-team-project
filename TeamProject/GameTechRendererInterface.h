#pragma once

#include "DecalSystem.h"
#include <string>
#include <vector>
#include <memory>
#include "PointLight.h"
#include "LaserObject.h"

#include "Vector.h"

namespace NCL::Rendering {
	class Mesh;
	class Texture;
	class Shader;
	class RendererBase;
}

namespace NCL {
	class Camera;
	class Window;
}

namespace NCL::CSC8503 {
	class GameWorld;
	class RenderObject;

	struct UiSprite {
		Maths::Vector2 position;
		Maths::Vector2 size;
		Maths::Vector4 color;
		std::shared_ptr<Rendering::Texture> texture;
	};

	struct UiText {
		Maths::Vector2 position;
		std::string text;
		Maths::Vector4 color;
		float scale = 1.0f;
	};

	struct UIBox {
		Maths::Vector2 position;
		Maths::Vector2 size;
	};

	struct Button {
		Maths::Vector2 position;
		Maths::Vector2 size;
		Maths::Vector4 color;
	};

	struct Text {
		Maths::Vector2 position;
		std::string text;
		Maths::Vector4 color = Maths::Vector4(1, 1, 1, 1);
	};

    class UiElement {
    public:
        virtual void render(std::vector<UiSprite>& sprites) = 0;
		
		virtual void render(std::vector<UiText>& Uitexts) = 0;
        
		virtual void Animate(float dt) = 0;

        bool IsActive() const {
            return enabled;
        }

        void SetActive(bool active) {
            enabled = active;
        }

    protected:
        bool enabled = false;
    };

	class GameTechRendererInterface
	{
	public:
		virtual Rendering::RendererBase* getBase() = 0;
		virtual void drawFrame(float dt) = 0;

		GameTechRendererInterface(Window* window);
		virtual ~GameTechRendererInterface() = default;

		virtual Rendering::Mesh* LoadMesh(const std::string& name) = 0;
		virtual Rendering::Texture*	LoadTexture(const std::string& name) = 0;

        // Get UI Elements
        std::vector<UiElement*> GetUiElements() {
            return uiElements;
        }

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
		void SetVignetteIntesnity(float intensityIn) {
			vignetteIntensity = intensityIn;
		}

		void SetDelta(float dt) {
			delta = dt;
		}

		/*
		int GetCurrentFrame() const {
			return currentFrame;
		}

		void SetCurrentFrame(int value) {
			currentFrame = value;
		}

		float GetFrameTime() const {
			return frameTime;
		}

		void SetFrameTime(float value) {
			frameTime = value;
		}*/
	
		void ClearUIElemets() {
			uiElements.clear();
		}

	protected:
		// Post-processing settings
		bool hdrOn = true;
		std::vector<UiElement*> uiElements;
		bool vignetteOn = false;
		float vignettePulse = 0;
		float vignetteIntensity = 0;
		btVector3 vignetteColour = btVector3(0.05f, 0.0f, 0.0f);
		float delta = 0;
		Window* window;
		Camera* camera = nullptr;

		std::vector<RenderObject*> frameObjects;
        std::vector<UiSprite> frameSprites;
		std::vector<UiText> frameTexts;
		std::vector<PointLight*> lights;

		std::vector<LaserObject*> lasers;
		DecalSystem decalSystem;

		//Mesh Animation additions:
		//int currentFrame = 0;
		//float frameTime = 0.0f;
	};
}

