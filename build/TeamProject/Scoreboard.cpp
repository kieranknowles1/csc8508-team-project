#include "Scoreboard.h"

namespace NCL::CSC8503 {

	void Scoreboard::AddPlayer(Lobbies::User user) {
		players.insert(std::make_pair(user.GetUserID(), Player{ user, TeamColor::BLUE, 0 }));
	}

	void Scoreboard::SetColor(Lobbies::User user) {

		players[user.GetUserID()].color = TeamColor::GREEN;
	}

	void Scoreboard::SetScore() {
		for (auto& player : players) {
			player.second.score ++;
		}
	}

	void Scoreboard::UpdateScoreboard() {
		SortPlayers();
	}

	void Scoreboard::SortPlayers() {
		std::sort(players.begin(), players.end(), [](const auto& a, const auto& b) {
			return a.second.score > b.second.score;
			});
	}
}