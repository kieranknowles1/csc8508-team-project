#include "EndScreenMP.h"

EndScreenMPUI::EndScreenMPUI()
{
	InitMenu();
	PopulateLeaderboard();
}

void EndScreenMPUI::render(std::vector<UiSprite>& sprites)
{
	sprites.push_back({ background.position, background.size, backgroundColor });

	for (const auto& box : boxes) {
		sprites.push_back({ box.position, box.size, borderColor });
		sprites.push_back({ box.position, {box.size.x - borderThickness, box.size.y - borderThickness}, boxColor});
	}

	for (auto& button : buttons)
	{
		sprites.push_back({ button.position, button.size, activeButton });
	}
}

void EndScreenMPUI::render(std::vector<UiText>& texts)
{
	for (auto& text : buttonTexts)
	{
		texts.push_back({ text.position, text.text, text.color });
	}
	for (auto& text : leaderboardTexts)
	{
		texts.push_back({ text.position, text.text, textColor });
	}
}

void EndScreenMPUI::InitMenu()
{
	background = { Vector2(0.5f, 0.5f), Vector2(1, 1) };
	//created buttons
	buttons.push_back({ Vector2(0.5f, 0.1f), buttonSize });

	buttonTexts.push_back({ Vector2(0.45f - buttonSize.x / 3.5, 0.9f + buttonSize.y / 4), "Return to Main Menu" });

	//created leaderboard
	Vector2 boxSize = { leaderboardSize.x / columns, leaderboardSize.y / rows };

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			
			Vector2 position = { 0.5f - leaderboardSize.x / 2.0f + boxSize.x / 2.0f + j * boxSize.x,
								 0.6f - leaderboardSize.y / 2.0f + boxSize.y / 2.0f + i * boxSize.y };
			
			boxes[i * columns + j] = { position, boxSize };

			Vector2 textPosition = { position.x - (boxSize.x / 2.0f) + 0.01f, position.y - boxSize.y * 2.2f };
			leaderboardTexts[i * columns + j] = { textPosition, "" };
		}
		
		if (i == 0) {
			leaderboardTexts[i * columns].text = "Player:";
			leaderboardTexts[i * columns + 1].text = "Color:";
			leaderboardTexts[i * columns + 2].text = "Score:";
		}
	}
}

void EndScreenMPUI::PopulateLeaderboard()
{
	SortPlayers();
	for (size_t i = 0; i < players.size(); ++i) {
		leaderboardTexts[i * columns + 3].text = players[i].name;
		leaderboardTexts[i * columns + 4].text = TeamColorToString(players[i].color);
		leaderboardTexts[i * columns + 5].text = std::to_string(players[i].score);
	}
}