#pragma once

#include <unordered_map>

#include "Multiplayer/User.hpp"
#include "Colors.h"

namespace Lobbies {
	/**
	 * @brief Color Selection Class.
	 * 
	 * Assigns a user to the color. Only one user can be assigned.
	 */
	class UserColor {
	public:
		/**
		 * @brief UserColor Constructor.
		 * 
		 * @param color The color this UserColor will represent.
		 */
		UserColor(TeamColor color) : m_color(color) {}

		/**
		 * @brief Assign a user to be in control of this player.
		 * 
		 * If there is already a user in control, this function will not allow
		 * the new player to be assigned. The old user must be removed first.
		 * 
		 * @param user The user to assume control.
		 * @return true if the user was allowed to be assigned, otherwise false.
		 */
		bool SetUser(User user);

		/**
		 * @brief Remove the current user's control.
		 */
		inline void Clear() { m_assigned.reset(); }

		/**
		 * @brief Get the user in control.
		 * 
		 * @return std::optional container. If there is a user assigned, it will be inside.
		 */
		inline std::optional<User> GetUser() { return m_assigned; }

	private:
		TeamColor m_color;
		std::optional<User> m_assigned;
	};

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
		Lobby(User hostUser, int maxSize) : m_hostUser(hostUser), m_maxSize(maxSize) {}

		/**
		 * Fetch the maximum size of the lobby.
		 * 
		 * @return The maximum size of the lobby.
		 */
		inline int GetLobbySize() const { return m_maxSize; }

		/**
		 * @brief Add a user to the lobby.
		 * @param user The user to insert.
		 * @return true if the user was added, otherwise false.
		 */
		bool AddUser(User user);

		/**
		 * @brief Remove a user from the lobby.
		 * 
		 * If the user is not in the lobby, this function does NOT throw an
		 * exception.
		 * 
		 * @param user The user to remove.
		 */
		inline void RemoveUser(User* user) { m_players.erase(user->GetUserID()); }

	private:
		User m_hostUser;
		const int m_maxSize;

		std::unordered_map<unsigned int, User> m_players;
	};
}
