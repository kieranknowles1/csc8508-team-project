#pragma once

#include <Colors.h>
#include "Multiplayer/User.hpp"
#include "GameTechRendererInterface.h"
#include "Multiplayer/Lobby.hpp"

using namespace NCL::Maths;

struct ScoreboardBoxes {
    Vector2 position;
    Vector2 size;
};

struct ScoreboardText {
    Vector2 position;
    std::string text;
};

struct Player {
    std::string name;
    int score = 0;
    TeamColor color;
    unsigned int playerID;
	unsigned int ping = 0;
};

class Scoreboard : public UiElement {
public:
    Scoreboard();

    void InitScoreboard();
    void UpdateScoreboardText();
    void UpdatePing();
    void PopulateScoreboard();

    void render(std::vector<UiSprite>& sprites) override;
    void render(std::vector<UiText>& texts) override;
    void Animate(float dt) override {}

    void AddPlayer(Player playerData) {
        players.push_back(playerData);
    }

    void SetScore(unsigned int score, unsigned int playerId) {
        for (auto& player : players) {
            if (player.playerID == playerId) {
                player.score = score;
            }
        }
    }

    void SetColor(unsigned int playerID, TeamColor color) {
        for (auto& player : players) {
            if (player.playerID == playerID) {
                player.color = color;
            }
        }
    }

private:
    Vector2 screenCenter = Vector2(0.5f, 0.5f);
    Vector2 scoreboardSize = Vector2(0.7f, 0.7f);
    std::array<ScoreboardBoxes, 36> boxes;
    std::array<ScoreboardText, 36> Uitexts;
    std::vector<Player> players;
    int columns = 4;
    int rows = 9;

    std::string TeamColorToString(TeamColor color) {
        switch (color) {
        case TeamColor::RED:
            return "Red";
        case TeamColor::BLUE:
            return "Blue";
        case TeamColor::GREEN:
            return "Green";
        case TeamColor::YELLOW:
            return "Yellow";
        case TeamColor::ORANGE:
            return "Orange";
        case TeamColor::PURPLE:
            return "Purple";
        case TeamColor::PINK:
            return "Pink";
        case TeamColor::CYAN:
            return "Cyan";
        default:
            return "Unknown";
        }
    }

    void SortPlayers() {
        std::sort(players.begin(), players.end(), [](const Player& a, const Player& b) {
            return a.score > b.score;
            });
    }

};
