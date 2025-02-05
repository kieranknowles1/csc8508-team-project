#include "Network.hpp"
#include "Server.hpp"


Server::~Server() {
	if (m_thread != nullptr) delete m_thread;
}


void Server::Start(bool threaded) {
	m_stateMutex.lock();

	if (m_state == ServerState::ERRORED) {
		m_stateMutex.unlock();
		return;
	}

	m_state = ServerState::STARTING;

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = m_port;

	if (!CreateHost(&address, MAX_CLIENTS, static_cast<size_t>(Channel::CHANNEL_COUNT), 0, 0)) {
		m_state = ServerState::ERRORED;
	}
	m_stateMutex.unlock();


	if (threaded) {

	}


	m_stateMutex.lock();
	if (m_state == ServerState::STARTING) {
		m_state = ServerState::RUNNING;
	}

#ifdef NETWORK_TEST
	if (m_state == ServerState::RUNNING) DebugOut("Server Started Successfully");
#endif

	m_stateMutex.unlock();

}


void Server::Stop() {

}

