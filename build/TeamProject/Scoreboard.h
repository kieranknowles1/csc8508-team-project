#pragma once

#include <unordered_map>
#include <Colors.h>

#include "Multiplayer/User.hpp"
#include "GameTechRendererInterface.h"
#include "Multiplayer/Lobby.hpp"

using namespace NCL::Maths;


struct ScoreboardBoxes {
	Vector2 position;
	Vector2 size;
};

struct Player {
	std::string name;
	int score = 0;
	std::string color;
};


class Scoreboard : public UiElement{
public:
	Scoreboard();
	void render(std::vector<UiSprite>& sprites) override;
	void render(std::vector<UiText>& texts) override;

	void AddPlayer(Player playerData) {
		players.push_back(playerData);
	};

	void SetColor();
	void SetScore();
	void SortPlayers();
	void UpdateScoreboardData();

private:
	Vector2 screenCenter = Vector2(0.5f, 0.5f);
	Vector2 scoreboardSize = Vector2(0.7f, 0.7f);
	std::array<ScoreboardBoxes, 27> boxes;
	std::vector<Player> players;
	int columns = 3;
	int rows = 9;
};
