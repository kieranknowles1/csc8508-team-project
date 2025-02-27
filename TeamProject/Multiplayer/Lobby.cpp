#include "Lobby.hpp"



namespace Lobbies {
	bool UserColor::SetUser(const User& user) {
		if (m_assigned.has_value()) return false;
		m_assigned.emplace(user);
	}



	bool Lobby::AddUser(const User& user) {
		if (m_players.size() >= m_maxSize) return false;
		m_players.insert(std::make_pair(user.GetUserID(), user));
	}


}
