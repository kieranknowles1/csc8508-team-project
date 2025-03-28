#include "Scoreboard.h"

Scoreboard::Scoreboard() {
    InitScoreboard();
}

void Scoreboard::UpdateScoreboardText() {
    SortPlayers();
    for (size_t i = 0; i < players.size(); ++i) {
        Uitexts[i * columns + 4].text = players[i].name;
        Uitexts[i * columns + 5].text = std::to_string(players[i].score);
        Uitexts[i * columns + 6].text = TeamColorToString(players[i].color);
		Uitexts[i * columns + 7].text = std::to_string(players[i].ping);
    }
}

void Scoreboard::render(std::vector<UiSprite>& sprites) {
    static const Vector4 scoreboardColor(0.4f, 0.4f, 0.4f, 0.4f);
    static const Vector4 borderColor(0.1f, 0.1f, 0.1f, 1.0f);
    const float borderThickness = 0.005f;

    for (const auto& box : boxes) {
        sprites.push_back({ box.position, box.size, scoreboardColor });

        Vector2 topLeft = box.position - box.size * 0.5f;
        Vector2 bottomRight = box.position + box.size * 0.5f;

        sprites.push_back({ {box.position.x, topLeft.y + borderThickness * 0.5f}, {box.size.x, borderThickness}, borderColor });
        sprites.push_back({ {box.position.x, bottomRight.y - borderThickness * 0.5f}, {box.size.x, borderThickness}, borderColor });
        sprites.push_back({ {topLeft.x + borderThickness * 0.5f, box.position.y}, {borderThickness, box.size.y}, borderColor });
        sprites.push_back({ {bottomRight.x - borderThickness * 0.5f, box.position.y}, {borderThickness, box.size.y}, borderColor });
    }
}

void Scoreboard::render(std::vector<UiText>& texts) {
    static const Vector4 textColor(1, 1, 1, 1);
    for (const auto& text : Uitexts) {
        texts.push_back({ text.position, text.text, textColor });
    }
}

void Scoreboard::PopulateScoreboard() {
	players.clear();
	for (unsigned int i = 1; i < 9; ++i) {
		Player player;
		player.name = "Player " + std::to_string(i);
		player.score = 0;
		player.color = colors[(i - 1)];
		player.playerID = i;
		player.ping = 0;
		players.push_back(player);
	}
}

void Scoreboard::InitScoreboard() {
	PopulateScoreboard();
    Vector2 boxSize = { scoreboardSize.x / columns, scoreboardSize.y / 9 };

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            Vector2 position = { screenCenter.x - scoreboardSize.x / 2.0f + boxSize.x / 2.0f + j * boxSize.x, 
                                 screenCenter.y - scoreboardSize.y / 2.0f + boxSize.y / 2.0f + i * boxSize.y };
            boxes[i * columns + j] = { position, boxSize };

            Vector2 textPosition = { position.x - (boxSize.x / 2.0f) + 0.01f, position.y + 0.02f};
            Uitexts[i * columns + j] = { textPosition, "" };
        }
        if (i == 0) {
            Uitexts[i * columns].text = "Player:";
            Uitexts[i * columns + 1].text = "Score:";
            Uitexts[i * columns + 2].text = "Color:";
			Uitexts[i * columns + 3].text = "Ping:";
        }
    }
    //UpdatePing();
    UpdateScoreboardText();
}

void Scoreboard::UpdatePing() {
	
}