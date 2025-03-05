#pragma once

#include <memory>

class Network;

namespace Lobbies {
	/**
	 * @brief User class containing information about players on the network.
	 */
	class User {
	public:
		/**
		 * @brief User Constructor.
		 * 
		 * Assigns the userID to the User.
		 * 
		 * @param userID the ID the user will use.
		 */
		User(unsigned int userID) : m_userID(userID) {}

		/**
		 * @brief Fetch the user's ID.
		 */
		inline unsigned int GetUserID() const { return m_userID; }

		/**
		 * @brief Serialize this object.
		 */
		std::unique_ptr<char[]> Serialize() {
			std::unique_ptr<char[]> data = std::make_unique<char[]>(sizeof(m_userID));
			std::memcpy(data.get(), &m_userID, sizeof(unsigned int));
			return data;
		}

	private:
		unsigned int m_userID = -1;
	};
}
