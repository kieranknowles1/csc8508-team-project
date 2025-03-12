#include "Scoreboard.h"


Scoreboard::Scoreboard() {
	UpdateScoreboardData();
}

void Scoreboard::render(std::vector<UiSprite>& sprites) {
	Vector4 scoreboardColor = Vector4(0.1f, 0.1f, 0.1f, 0.6f);
	for (const auto& box : boxes) {
		sprites.push_back({ box.position, box.size, scoreboardColor });
	}

}

void Scoreboard::render(std::vector<UiText>& texts) {
	Vector4 textColor = Vector4(1, 1, 1, 1);
	for (const auto& text : Uitexts) {
		texts.push_back({ text.position, text.text, textColor });
	}
}

void Scoreboard::SortPlayers() {
	std::vector<std::pair<unsigned int, Player>> playerVector(players.begin(), players.end());

	// Sort the vector based on the player's score in descending order
	std::sort(playerVector.begin(), playerVector.end(), [](const auto& a, const auto& b) {
		return a.second.score > b.second.score;
		});

	// Clear the original map and insert the sorted players back into it
	players.clear();
	for (const auto& pair : playerVector) {
		players.insert(pair);
	}
}

void Scoreboard::UpdateScoreboardData() {
	SortPlayers();
	Vector2 boxSize = Vector2(scoreboardSize.x/columns, scoreboardSize.y/9);
	
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			Vector2 position = Vector2(screenCenter.x - scoreboardSize.x / 2.0f + boxSize.x / 2.0f + j * boxSize.x, screenCenter.y - scoreboardSize.y / 2.0f + boxSize.y / 2.0f + i * boxSize.y);
			boxes[i * columns + j] = { position, boxSize };

			Uitexts[i * columns + j] = { position, ""};
		}
	}

	for (int i = 0; i < rows; i++) {
		if (i == 0) {
			Uitexts[i].text = "Player:";
			Uitexts[i + 1].text = "Score:";
			Uitexts[i + 2].text = "Color:";
		}
		else {
			Uitexts[i * columns].text = players[i - 1].name;
			Uitexts[i * columns + 1].text = std::to_string(players[i - 1].score);
			Uitexts[i * columns + 2].text = TeamColorToString(players[i - 1].color);
		}
	}
}