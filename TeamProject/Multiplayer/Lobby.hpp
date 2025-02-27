#pragma once

#include <unordered_map>

#include "Multiplayer/User.hpp"

namespace Lobbies {
	/**
	 * @brief Lobby class that contains a collection of user IDs to separate
	 * who is who.
	 */
	class Lobby {
	public:
		/**
		 * @brief Construct Lobby with host User.
		 * 
		 * @param hostID The User who is in charge of the lobby.
		 */
		Lobby(User hostUser) : m_hostUser(hostUser) {}

	private:
		User m_hostUser;
		std::unordered_map<int, User> m_players;
	};
}
