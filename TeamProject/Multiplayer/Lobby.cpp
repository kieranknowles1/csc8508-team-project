#include "Lobby.hpp"



namespace Lobbies {
	bool UserColor::SetUser(const User& user) {
		if (m_assigned.has_value()) return false;
		m_assigned.emplace(user);
	}


	Lobby::Lobby(User hostUser, int maxSize) : m_hostUser(hostUser), m_maxSize(maxSize) {
		uint8_t value = 0;
		std::generate(m_userColors.begin(), m_userColors.end(), [&value] {
			return UserColor(static_cast<TeamColor>(value++));
			});
	}


	bool Lobby::AddUser(const User& user) {
		if (m_players.size() >= m_maxSize) return false;
		m_players.insert(std::make_pair(user.GetUserID(), user));
	}

	bool Lobby::AreAllPlayersAssignedColor() const {
		int total = 0;
		for (const UserColor& color : m_userColors) {
			if (color.GetUser().has_value()) ++total;
		}
		return total == m_players.size();
	}


}
