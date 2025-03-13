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
};


class Scoreboard : public UiElement{
public:
	Scoreboard();
	void render(std::vector<UiSprite>& sprites) override;
	void render(std::vector<UiText>& texts) override;
	void Animate(float dt) override {};

	void AddPlayer(unsigned int playerID, Player playerData) {
		players.insert(std::pair<unsigned int, Player>(playerID, playerData));
	};

	void SetScore(unsigned int score, unsigned int playerId) {
		players[playerId].score = score;
	}

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

	void SetColor(unsigned int playerID, TeamColor color) {
		players[playerID].color = color;
	}
	
	void SortPlayers();
	void UpdateScoreboardData();

private:
	Vector2 screenCenter = Vector2(0.5f, 0.5f);
	Vector2 scoreboardSize = Vector2(0.7f, 0.7f);
	std::array<ScoreboardBoxes, 27> boxes;
	std::array<ScoreboardText, 27> Uitexts;
	std::unordered_map<unsigned int, Player> players;
	int columns = 3;
	int rows = 9;
};
