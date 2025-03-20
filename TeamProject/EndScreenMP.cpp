#include "EndScreenMP.h"

EndScreenUI::EndScreenUI()
{
	InitMenu();

	AddPlayer({ "Player1", 100, TeamColor::RED, 1 });
	AddPlayer({ "Player2", 200, TeamColor::BLUE, 2 });
	AddPlayer({ "Player3", 300, TeamColor::GREEN, 3 });

	PopulateLeaderboard();
}

void EndScreenUI::render(std::vector<UiSprite>& sprites)
{
	Vector4 backgroundColor = Vector4(0.1, 0.1, 0.1, 1);
	Vector4 buttonColor = Vector4(0.4f, 0.4f, 0.4f, 1);
	Vector4 borderColor = Vector4(0, 0, 0, 1);
	Vector4 boxColor = Vector4(0.4f, 0.4f, 0.4f, 1);
	float borderThickness = 0.005f;

	sprites.push_back({ background.position, background.size, backgroundColor });

	for (const auto& box : boxes) {
		sprites.push_back({ box.position, box.size, borderColor });
		sprites.push_back({ box.position, {box.size.x - borderThickness, box.size.y - borderThickness}, boxColor});
	}

	for (auto& button : buttons)
	{
		sprites.push_back({ button.position, button.size, button.color });
	}
}

void EndScreenUI::render(std::vector<UiText>& texts)
{
	Vector4 textColor = Vector4(0, 0, 0, 1);

	for (auto& text : buttonTexts)
	{
		texts.push_back({ text.position, text.text, text.color });
	}
	for (auto& text : leaderboardTexts)
	{
		texts.push_back({ text.position, text.text, textColor });
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
	Vector2 boxSize = { leaderboardSize.x / columns, leaderboardSize.y / rows };

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			
			Vector2 position = { 0.5f - leaderboardSize.x / 2.0f + boxSize.x / 2.0f + j * boxSize.x,
								 0.6f - leaderboardSize.y / 2.0f + boxSize.y / 2.0f + i * boxSize.y };
			
			boxes[i * columns + j] = { position, boxSize };

			Vector2 textPosition = { position.x - (boxSize.x / 2.0f) + 0.01f, position.y - 0.2f };
			leaderboardTexts[i * columns + j] = { textPosition, "" };
		}
		
		if (i == 0) {
			leaderboardTexts[i * columns].text = "Player:";
			leaderboardTexts[i * columns + 1].text = "Score:";
			leaderboardTexts[i * columns + 2].text = "Color:";
		}
	}
}

void EndScreenUI::UpdateMenu(unsigned int selection)
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

void EndScreenUI::PopulateLeaderboard()
{
	SortPlayers();
	for (size_t i = 0; i < players.size(); ++i) {
		leaderboardTexts[i * columns + 3].text = players[i].name;
		leaderboardTexts[i * columns + 4].text = std::to_string(players[i].score);
		leaderboardTexts[i * columns + 5].text = TeamColorToString(players[i].color);
	}
}