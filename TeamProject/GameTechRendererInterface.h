#pragma once

#include "DecalSystem.h"
#include <string>
#include <vector>
#include <memory>

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
	class Scoreboard;

	struct UIElement {
		// TODO: Need a UV field
		Maths::Vector2 position;
		Maths::Vector2 size;
		Maths::Vector4 color;
		std::shared_ptr<Rendering::Texture> texture;
		bool isScoreboard = false;
	};

	struct UITextElement {
		Maths::Vector2 position;
		Maths::Vector4 color;
		std::string text;
		bool isScoreboard = false;
	};

	struct PlayerData {
		std::string name;
		std::string color;
		int score;
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

		void AddUIElement(const UIElement& element) {
			uiElements.push_back(element);
		}

		void AddUITextElement(const UITextElement& element) {
			uiTextElements.push_back(element);
		}

		void AddPlayerData(const PlayerData& data) {
			playerData.push_back(data);
		}

		void ToggleScoreboard() {
			showScoreboard = !showScoreboard;
		}

		// TODO: Proper UI class
		void initUi();

	protected:
		//adding bools to toggle post processing. Must be accessible from the specific renderer
		bool hdrOn = true;
		bool vignetteOn = false;
		float vignettePulse = 0;
		Window* window;
		Camera* camera = nullptr;
		std::vector<UIElement> uiElements;
		std::vector<UITextElement> uiTextElements;
		std::vector<PlayerData> playerData;
		std::vector<RenderObject*> frameObjects;
		DecalSystem decalSystem;
		bool showScoreboard = false;
		Maths::Vector2 screenCenter = Maths::Vector2(0.5f, 0.5f);
	};
}

