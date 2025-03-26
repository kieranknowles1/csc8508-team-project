#include "MainMenuScreen.h"

MainMenuUI::MainMenuUI() {
	InitMenu();
}

void MainMenuUI::render(std::vector<UiSprite>& sprites)
{
	Vector4 backgroundColor = Vector4(0, 0, 0, 1);
	
	sprites.push_back({ background.position, background.size, backgroundColor });

	for (auto& button : buttons)
	{
		sprites.push_back({ button.position, button.size, button.color });
	}
}

void MainMenuUI::render(std::vector<UiText>& texts)
{
	Vector4 textColor = Vector4(0, 0, 0, 1);

	for (auto& text : buttonTexts)
	{
		texts.push_back({ text.position, text.text, text.color });
	}
}


void MainMenuUI::InitMenu() {
	
}

void MainMenuUI::UpdateMenu(unsigned int selection) {

}