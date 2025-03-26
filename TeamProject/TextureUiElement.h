#pragma once
#include "GameTechRendererInterface.h"

namespace NCL::CSC8503 {


	class TextureUiElement : public UiElement {
	public:
		TextureUiElement(UiSprite sprite) : sprite(sprite) {}

		void render(std::vector<UiSprite>& sprites) override {
			
			sprites.push_back(sprite);
		}

		void render(std::vector<UiText>& texts) override {}

		void Animate(float dt) override {}

	private:
		UiSprite sprite;
	};
}
