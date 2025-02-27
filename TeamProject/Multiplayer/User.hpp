#pragma once

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
	private:
		unsigned int m_userID = -1;
	};
}
