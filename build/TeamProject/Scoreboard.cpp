#include "Scoreboard.h"


Scoreboard::Scoreboard() {
	UpdateScoreboardData();
}

void Scoreboard::render(std::vector<UiSprite>& sprites) {
	Vector4 scoreboardColor = Vector4(0.1f, 0.1f, 0.1f, 0.6f);

}

void Scoreboard::render(std::vector<UiText>& texts) {
	Vector4 textColor = Vector4(1, 1, 1, 1);
	
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
			boxes[i * columns + j] = { Vector2(screenCenter.x - scoreboardSize.x / 2.0f + boxSize.x / 2.0f + j * boxSize.x, screenCenter.y - scoreboardSize.y / 2.0f + boxSize.y / 2.0f + i * boxSize.y), boxSize };
		}
	}
}