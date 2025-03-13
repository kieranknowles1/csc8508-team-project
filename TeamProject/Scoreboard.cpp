#include "Scoreboard.h"

Scoreboard::Scoreboard() {
    AddPlayer({ "Player1", 0, TeamColor::RED, 0});
    AddPlayer({ "Player2", 3, TeamColor::BLUE, 1 });
    AddPlayer({ "Player3", 6, TeamColor::GREEN, 2 });
    AddPlayer({ "Player4", 4, TeamColor::YELLOW, 3 });
    InitScoreboard();
}

void Scoreboard::SortPlayers() {
    // Sort the vector based on the player's score in descending order
    std::sort(players.begin(), players.end(), [](const Player& a, const Player& b) {
        return a.score > b.score;
        });
}

void Scoreboard::UpdateScoreboardText() {
    // Debug print to check player data
    SortPlayers();
    std::cout << players[0].name << std::endl;
    for (int i = 0; i < players.size(); i++) {
        std::cout << players[i].name << std::endl;
        Uitexts[i * columns + 3].text = players[i].name;
        std::cout << players[i].name << std::endl;
        Uitexts[i * columns + 4].text = std::to_string(players[i].score);
        Uitexts[i * columns + 5].text = TeamColorToString(players[i].color);
    }
}

void Scoreboard::render(std::vector<UiSprite>& sprites) {
    static const Vector4 scoreboardColor(0.3f, 0.3f, 0.3f, 0.4f);
    for (const auto& box : boxes) {
        sprites.push_back({ box.position, box.size, scoreboardColor });
    }
}

void Scoreboard::render(std::vector<UiText>& texts) {
    static const Vector4 textColor(1, 0.4f, 0, 0.6f);
    for (const auto& text : Uitexts) {
        texts.push_back({ text.position, text.text, textColor });
    }
}

void Scoreboard::InitScoreboard() {
    Vector2 boxSize = Vector2(scoreboardSize.x / columns, scoreboardSize.y / 9);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            Vector2 position = Vector2(screenCenter.x - scoreboardSize.x / 2.0f + boxSize.x / 2.0f + j * boxSize.x, screenCenter.y - scoreboardSize.y / 2.0f + boxSize.y / 2.0f + i * boxSize.y);
            boxes[i * columns + j] = { position, boxSize };

            Vector2 textPosition = Vector2(position.x - (boxSize.x / 2.0f) + 0.01f, position.y);
            Uitexts[i * columns + j] = { textPosition, "" };
        }
        if (i == 0) {
            Uitexts[i * columns].text = "Player:";
            Uitexts[i * columns + 1].text = "Score:";
            Uitexts[i * columns + 2].text = "Color:";
        }
    }
    UpdateScoreboardText();
}