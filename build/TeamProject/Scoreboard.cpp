#include "Scoreboard.h"
#include <vector>
#include <algorithm>

namespace NCL::CSC8503 {

    void Scoreboard::AddPlayer(Lobbies::User user) {
        Player player(user, TeamColor::BLUE, 0);
        players.insert(std::make_pair(user.GetUserID(), player));
    }

    void Scoreboard::SetColor(Lobbies::User user) {
        players.at(user.GetUserID()).color = TeamColor::GREEN;
    }

    void Scoreboard::SetScore() {
        for (auto& player : players) {
            player.second.score++;
        }
    }

    void Scoreboard::UpdateScoreboard() {
        SortPlayers();
    }

    void Scoreboard::SortPlayers() {
        std::vector<std::pair<unsigned int, Player>> playerVector(players.begin(), players.end());
        std::sort(playerVector.begin(), playerVector.end(), [](const auto& a, const auto& b) {
            return a.second.score > b.second.score;
        });

        players.clear();
        for (const auto& pair : playerVector) {
            players.insert(pair);
        }
    }
}
