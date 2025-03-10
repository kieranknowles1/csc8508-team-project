#pragma once

#include <unordered_map>
#include <Colors.h>

#include "Multiplayer/User.hpp"
#include "GameTechRendererInterface.h"

namespace Lobbies {
	class User;
}
namespace NCL::CSC8503 {
	class GameTechRendererInterface;
	class Scoreboard {
	public:
		struct Player {
			Lobbies::User user;
			TeamColor color;
			int score;
		};

		struct ScoreboardText {
			Maths::Vector2 position;
			std::string text;
			Maths::Vector4 color = Maths::Vector4(0, 0, 0, 1);
			bool isScoreboard = false;
		};

		std::unordered_map<unsigned int, struct Player> players;

		void AddPlayer(Lobbies::User user);
		void SetColor(Lobbies::User user);
		void SetScore();
		void UpdateScoreboard();
		void InitScoreboardUI();

		void AddScoreboardText(const ScoreboardText& element) {
			scoreboardTextElements.push_back(element);
		}

	private:
		void SortPlayers();
		void UpdateTextValues();

		std::vector<ScoreboardText> scoreboardTextElements;
	};
}
