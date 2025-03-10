#pragma once

#include <unordered_map>
#include <Colors.h>

#include "Multiplayer/User.hpp"

namespace Lobbies {
	class User;
}
namespace NCL::CSC8503 {
	class Scoreboard {
	public:
		struct Player {
			Lobbies::User user;
			TeamColor color;
			int score;
		};

		std::unordered_map<unsigned int, struct Player> players;

		void AddPlayer(Lobbies::User user);
		void SetColor(Lobbies::User user);
		void SetScore();
		void UpdateScoreboard();

	private:
		void SortPlayers();
	};
}
