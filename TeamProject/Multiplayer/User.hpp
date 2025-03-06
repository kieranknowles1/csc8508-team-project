#pragma once

#include <memory>
#include <string>
#include <string.h>

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
		 * The user ID is required to make a user to prevent changing what
		 * should be constant state.
		 * 
		 * @param userID the ID the user will use.
		 * @param name the display name of the user.
		 */
		User(unsigned int userID, std::string name = "") : m_userID(userID), m_name(name) {}

		/**
		 * @brief Fetch the user's ID.
		 */
		inline unsigned int GetUserID() const { return m_userID; }

		/**
		 * @brief The display name of the user.
		 */
		inline std::string GetDisplayName() const { return m_name; }

		/**
		 * @brief Give the user a name.
		 */
		inline void SetDisplayName(const std::string& name) { m_name = name; }

		/**
		 * @brief Serialize this object.
		 */
		std::unique_ptr<char[]> Serialize() {
			std::unique_ptr<char[]> data = std::make_unique<char[]>(sizeof(const unsigned int) + (sizeof(char) * (m_name.length() + 1)));
			std::memcpy(data.get(), &m_userID, sizeof(const unsigned int));
			std::memcpy(data.get() + sizeof(const unsigned int), m_name.c_str(), sizeof(char) * (m_name.length() + 1));
			return data;
		}

		/**
		 * @brief Convert raw byte data into a User object. 
		 * @param data A byte buffer that MUST BE NULL TERMINATED.
		 */
		static User Deserialize(const char* data) {
			unsigned int userID;
			
			// Using whole length to make sure buffer size is sufficient.
			std::unique_ptr<char[]> name = std::make_unique<char[]>(strlen(data));

			memcpy(&userID, data, sizeof(unsigned int));
			memcpy(name.get(), data + sizeof(unsigned int), strlen(data + sizeof(unsigned int)) + 1);
			return User(userID, std::string(name.get()));
		}

	private:
		unsigned int m_userID;
		std::string m_name = "";
	};
}
