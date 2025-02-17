#include "Client.hpp"

#include <thread>
#include <chrono>
#include <iostream>

Client::Client() {
	CreateHost(nullptr, MAX_CLIENTS, 2, 0, 0);
}


Client::~Client() {}


void Client::ConnectTo(ENetAddress& destination) {
	m_server = enet_host_connect(m_host, &destination, 2, 0);
	if (m_server == nullptr) {
		return;
	}

	std::lock_guard<std::mutex> lock(m_connectionMutex);
	m_state = ConnectionState::WAITING;
}




void Client::Update() {
	ENetEvent event;
	bool running;
	int response;

	while (running) {
		response = enet_host_service(m_host, &event, EVENT_WAIT);

		if (response < 0) {
			m_connectionMutex.lock();
			enet_peer_disconnect(m_server, 0);
			m_state = ConnectionState::DISCONNECTED;
			m_connectionMutex.unlock();
		}
		else if (response > 0) {
			if (event.type == ENET_EVENT_TYPE_RECEIVE) {
				enet_packet_destroy(event.packet);
			}
		}

		m_connectionMutex.lock();
		if (m_state != ConnectionState::CONNECTED) {
			running = false;
		}
		m_connectionMutex.unlock();
	}
}
