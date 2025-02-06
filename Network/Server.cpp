#include "Network.hpp"
#include "Server.hpp"


Server::~Server() {
	if (m_thread != nullptr) {
		m_thread->join();
		delete m_thread;
	}
}


void Server::Start() {
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

	if (m_state == ServerState::STARTING) {
		m_state = ServerState::RUNNING;
	}
	m_stateMutex.unlock();

	m_thread = new std::thread(&Server::Run, this);

#ifdef NETWORK_TEST
	if (m_state == ServerState::RUNNING) DebugOut("Server Started Successfully");
#endif


}


void Server::Stop() {

}


void Server::Run() {
	bool running = true;

	while (running) {
		Update();

		// Update running.
		std::lock_guard<std::mutex> lock(m_stateMutex);
		if (m_state != ServerState::RUNNING) {
			running = false;
		}
	}
}

void Server::Update() {
	ENetEvent event = GetEvent(EVENT_WAIT);

	switch (event.type) {
	case ENET_EVENT_TYPE_CONNECT:
		AttemptConnection(event);
		break;
	case ENET_EVENT_TYPE_DISCONNECT:
		RemoveConnection(event);
		break;
	case ENET_EVENT_TYPE_RECEIVE:
		break;
	default:
		return;
	}
}


void Server::AttemptConnection(ENetEvent& event) {
	if (!allowConnections || m_numConnections >= MAX_CLIENTS) {
		enet_peer_reset(event.peer);
		return;
	}
	connections[m_numConnections++] = event.peer;
}


void Server::RemoveConnection(ENetEvent& event) {
	for (int i = 0; i < m_numConnections; ++i) {
		if (event.peer == connections[i]) {
			m_numConnections--;

			// Filling the gap if one was created.
			if (i < m_numConnections) {
				connections[i] = connections[m_numConnections];
			}
			connections[m_numConnections] = nullptr;
			return;
		}
	}
}



