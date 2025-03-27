#include "MainMenuScreen.h"

MainMenuUI::MainMenuUI(std::shared_ptr<NCL::Rendering::Texture> logo) {

	fmodLogoTex = logo;
	InitMenu();
}

void MainMenuUI::render(std::vector<UiSprite>& sprites)
{
	Vector4 backgroundColor = Vector4(0, 0, 0, 1);
	
	sprites.push_back({ background.position, background.size, backgroundColor });

	sprites.push_back({ fmodLogo.position, fmodLogo.size, Vector4(1,1,1,1), fmodLogoTex});
	
	for (auto& button : buttons)
	{
		sprites.push_back({ button.position, button.size, button.color });
	}
}

void MainMenuUI::render(std::vector<UiText>& texts)
{

	texts.push_back({title.position, title.text, title.color, 3});
	
	for (auto& text : buttonTexts)
	{
		texts.push_back({ text.position, text.text, text.color });
	}
}


void MainMenuUI::InitMenu() {
	
	Vector2 buttonSize = Vector2(0.5f, 0.1f);
	background = { Vector2(0.5f, 0.5f), Vector2(1, 1) };
	fmodLogo = { Vector2(0.95f, 0.05f), Vector2(0.1f, 0.1f)};
	title = { Vector2(0.095f, 0.07f), "BlitzBox", Vector4(0.5, 0.5, 0.5, 1) };

	buttons.push_back({ Vector2(0.5f, 0.6f), buttonSize, inactiveButton });
	buttons.push_back({ Vector2(0.5f, 0.5f), buttonSize, inactiveButton });
	buttons.push_back({ Vector2(0.5f, 0.4f), buttonSize, inactiveButton });
	buttons.push_back({ Vector2(0.5f, 0.3f), buttonSize, inactiveButton });
	buttons.push_back({ Vector2(0.5f, 0.2f), buttonSize, inactiveButton });

	buttonTexts.push_back({ Vector2(0.4f, 0.4f + buttonSize.y / 5), "Single Player" });
	buttonTexts.push_back({ Vector2(0.42f, 0.5f + buttonSize.y / 5), "Host Game" });
	buttonTexts.push_back({ Vector2(0.423f, 0.6f + buttonSize.y / 5), "Join Game" });
	buttonTexts.push_back({ Vector2(0.44f, 0.7f + buttonSize.y / 5), "Credits" });
	buttonTexts.push_back({ Vector2(0.46f, 0.8f + buttonSize.y / 5), "Quit" });
}

void MainMenuUI::UpdateMenu(unsigned int selection) {
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