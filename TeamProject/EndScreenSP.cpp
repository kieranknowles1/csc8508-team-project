#include "EndScreenSP.h"

EndScreenSPUI::EndScreenSPUI()
{
	InitMenu();
}

void EndScreenSPUI::render(std::vector<UiSprite>& sprites)
{
	sprites.push_back({ background.position, background.size, backgroundColor });

	for (auto& button : buttons)
	{
		sprites.push_back({ button.position, button.size, activeButton });
	}
}

void EndScreenSPUI::render(std::vector<UiText>& texts)
{

	texts.push_back({ Vector2(0.118f, 0.15f), "Game Over", Vector4(1, 0, 0, 1), 2.5f });
	texts.push_back({ Vector2(0.43f, 0.5f), "Score: " + std::to_string(score), Vector4(1,1,1,1)});

	for (auto& text : buttonTexts)
	{
		texts.push_back({ text.position, text.text, text.color });
	}
}

void EndScreenSPUI::InitMenu()
{
	background = { Vector2(0.5f, 0.5f), Vector2(1, 1) };
	//created buttons
	buttons.push_back({ Vector2(0.5f, 0.2f), buttonSize });

	buttonTexts.push_back({ Vector2(0.45f - buttonSize.x / 3.5, 0.8f + buttonSize.y / 4), "Return to Main Menu" });
}