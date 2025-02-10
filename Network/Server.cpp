#include "Server.hpp"

#include <iostream>


Server::~Server() {
	if (m_thread != nullptr) {
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

}


void Server::Stop() {
	m_stateMutex.lock();
	m_state = ServerState::STOPPED;
	m_stateMutex.unlock();

	if (m_thread != nullptr) m_thread->join();
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

	if (event.type == ENET_EVENT_TYPE_CONNECT) {
		AttemptConnection(event);
	}
	else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
		RemoveConnection(event);
	}
	else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
		Packet::PacketBase packet = Packet::PacketBase::FromENetPacket(event.packet);
		enet_packet_destroy(event.packet);
		Handle(packet);
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



