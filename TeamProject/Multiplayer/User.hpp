#pragma once

class Network;

namespace Lobbies {
	class User {
	public:
		User(unsigned int userID) : m_userID(userID) {}
	private:
		unsigned int m_userID = -1;
	};
}
