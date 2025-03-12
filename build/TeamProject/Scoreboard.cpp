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

void Scoreboard::SetColor() {
}

void Scoreboard::SetScore() {
}

void Scoreboard::SortPlayers() {
	std::sort(players.begin(), players.end(), [](const Player& a, const Player& b) {
		return a.score > b.score; // Sort in descending order
		});
}

void Scoreboard::UpdateScoreboardData() {
	Vector2 boxSize = Vector2(scoreboardSize.x/columns, scoreboardSize.y/9);
	
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			boxes[i * columns + j] = { Vector2(screenCenter.x - scoreboardSize.x / 2.0f + boxSize.x / 2.0f + j * boxSize.x, screenCenter.y - scoreboardSize.y / 2.0f + boxSize.y / 2.0f + i * boxSize.y), boxSize };
		}
	}
}