#include "EndScreenMP.h"

EndScreenUI::EndScreenUI()
{
	InitMenu();
}

void EndScreenUI::render(std::vector<UiSprite>& sprites)
{
	Vector4 backgroundColor = Vector4(0, 0, 0, 1);
	Vector4 buttonColor = Vector4(0.4f, 0.4f, 0.4f, 1);
	Vector4 borderColor = Vector4(0, 0, 0, 1);
	Vector4 boxColor = Vector4(0.4f, 0.4f, 0.4f, 1);

	sprites.push_back({ background.position, background.size, backgroundColor });

	for (auto& button : buttons)
	{
		sprites.push_back({ button.position, button.size, button.color });
	}
	for (auto& leaderboard : leaderboardBorder)
	{
		sprites.push_back({ leaderboard.position, leaderboard.size, borderColor });
	}
	for (auto& leaderboard : leaderboard)
	{
		sprites.push_back({ leaderboard.position, leaderboard.size, boxColor });
	}
}

void EndScreenUI::render(std::vector<UiText>& texts)
{
	Vector4 textColor = Vector4(0, 0, 0, 1);

	for (auto& text : buttonTexts)
	{
		texts.push_back({ text.position, text.text, text.color });
	}
}

void EndScreenUI::InitMenu()
{
	Vector2 buttonSize = Vector2(0.3f, 0.075f);
	background = { Vector2(0.5f, 0.5f), Vector2(1, 1) };

	//created buttons
	buttons.push_back({ Vector2(0.3f, 0.1f), buttonSize });
	buttons.push_back({ Vector2(0.7f, 0.1f), buttonSize });

	buttonTexts.push_back({ Vector2(0.3f - buttonSize.x / 3.5, 0.9f + buttonSize.y / 8), "Main Menu" });
	buttonTexts.push_back({ Vector2(0.7f - buttonSize.x / 6, 0.9f + buttonSize.y / 8), "Quit" });

	//created leaderboard
	//Vector2 leaderboardSize = Vector2(0.7f, 0.7f);
	//Vector2 leaderboardPosition = Vector2(0.5f, 0.6f);
	//Vector2 borderSize = Vector2(0.7f, leaderboardSize.y / 9);
	//Vector2 rowSize = Vector2(borderSize.x - 0.005f, borderSize.y - 0.005f);

	//for (int i = 0; i < 9; i++)
	//{
	//	Vector2 boxPosition = Vector2(leaderboardPosition.x, (leaderboardPosition.y * 1.5f) - (i * borderSize.y));
	//	leaderboardBorder.push_back({ boxPosition, borderSize });
	//	leaderboard.push_back({ boxPosition, rowSize });

	//}
}

void EndScreenUI::UpdateMenu(int selection)
{
	for (int i = 0; i < buttons.size(); i++)
	{
		if (i == selection)
		{
			buttons[i].color = activeButton;
			buttonTexts[i].color = activeText;
		}
		else
		{
			buttons[i].color = inactiveButton;
			buttonTexts[i].color = inactiveText;
		}
	}
}