#pragma once

#include <array>
#include <unordered_map>
#include <optional>

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
		 * @brief Default Constructor
		 * 
		 * Required for std::array as it requires default constructible object.
		 */
		UserColor() : m_color(static_cast<TeamColor>(UINT8_MAX)) {}

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
		bool SetUser(const User& user);

		/**
		 * @brief Remove the current user's control.
		 */
		inline void Clear() { m_assigned.reset(); }

		/**
		 * @brief Get the user in control.
		 */
		inline std::optional<User> GetUser() const { return m_assigned; }

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
		Lobby(int maxSize);

		/**
		* Assign what user is in charge of the lobby.
		*/
		inline void SetHost(User hostUser) { m_hostUser.emplace(hostUser); }

		/**
		 * Fetch the maximum size of the lobby.
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
		inline void RemoveUser(const User& user) { m_players.erase(user.GetUserID()); }

		/**
		 * @brief Get an array of all the colors.
		 * 
		 * The returned array will contain assigned users IFF the UserColor
		 * has been assigned a user.
		 * 
		 * @see UserColor
		 * 
		 * @return std::array<UserColor> of colors.
		 */
		inline std::array<UserColor, 8> GetUserColors() const { return m_userColors; }

		/**
		 * @brief Determine if all the players currently in the lobby are assigned
		 * a color.
		 * 
		 * @return true if all the players are assigned a color, otherwise false. 
		 */
		bool AreAllPlayersAssignedColor() const;

		/**
		 * Attempt to assign a user to a color.
		 * 
		 * @param user The user to assign.
		 * @param color The color to assign the user to.
		 * @return true if the user was assigned, otherwise false.
		 */
		inline bool SetUserColor(User user, TeamColor color) { return m_userColors[static_cast<uint8_t>(color)].SetUser(user); }

	private:
		std::optional<User> m_hostUser;
		const int m_maxSize;

		std::array<UserColor, 8> m_userColors;
		std::unordered_map<unsigned int, User> m_players;
	};
}
