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
		sprites.push_back({ button.position, button.size, button.color });
	}
}

void EndScreenSPUI::render(std::vector<UiText>& texts)
{

	texts.push_back({ Vector2(0.118f, 0.15f), "Game Over", Vector4(1, 0, 0, 1), 2.5f });
	texts.push_back({ Vector2(0.43f, 0.5f), "Score: " + std::to_string(200), Vector4(1,1,1,1)});

	for (auto& text : buttonTexts)
	{
		texts.push_back({ text.position, text.text, text.color });
	}
}

void EndScreenSPUI::InitMenu()
{
	background = { Vector2(0.5f, 0.5f), Vector2(1, 1) };
	//created buttons
	buttons.push_back({ Vector2(0.3f, 0.1f), buttonSize });
	buttons.push_back({ Vector2(0.7f, 0.1f), buttonSize });

	buttonTexts.push_back({ Vector2(0.3f - buttonSize.x / 3.5, 0.9f + buttonSize.y / 4), "Main Menu" });
	buttonTexts.push_back({ Vector2(0.7f - buttonSize.x / 6, 0.9f + buttonSize.y / 4), "Quit" });
}

void EndScreenSPUI::UpdateMenu(unsigned int selection)
{
	for (int i = 0; i < buttons.size(); i++)
	{
		if (i == selection)
		{
			buttons[i].color = activeButton;
		}
		else
		{
			buttons[i].color = inactiveButton;
		}
	}
}