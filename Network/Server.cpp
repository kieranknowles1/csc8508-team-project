#include "Network.hpp"
#include "Server.hpp"


Server::~Server() {
	if (m_thread != nullptr) delete m_thread;
}


void Server::Start(bool threaded) {
	if (threaded) {
#ifdef NETWORK_TEST
		DebugOut("Starting network on a separate thread.");
#endif

	}

#ifdef NETWORK_TEST
	DebugOut("Starting network on the main thread.");
#endif
}


void Server::Stop() {
#ifdef NETWORK_TEST
	DebugOut("Stopping server.");
#endif


#ifdef NETWORK_TEST
	DebugOut("Server stopped.");
#endif
}


bool Server::CreateHost() {
	address.host = ENET_HOST_ANY;
	address.port = m_port;

	m_host = enet_host_create(&address, MAX_CLIENTS, static_cast<size_t>(Channel::CHANNEL_COUNT), 0, 0);

	if (m_host == nullptr) {
#ifdef NETWORK_TEST
		DebugOut("Host creation failed.")
#endif
		
		m_state = ServerState::ERRORED;
		return false;
	}
#ifdef NETWORK_TEST
	DebugOut("Host created.")
#endif

	return true;
}
