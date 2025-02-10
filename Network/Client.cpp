#include "Client.hpp"

#include <thread>
#include <chrono>
#include <iostream>

Client::Client() {
	CreateHost(nullptr, MAX_CLIENTS, 2, 0, 0);
}

Client::~Client() {
	
}


void Client::ConnectTo(ENetAddress& address) {
	ENetAddress destination;
	enet_address_set_host(&destination, "192.168.0.15");
	destination.port = 12345;

	m_server = enet_host_connect(m_host, &destination, 2, 0);
	if (m_server == nullptr) {
		return;
	}

	std::lock_guard<std::mutex> lock(m_connectionMutex);
	m_state = ConnectionState::WAITING;

	std::thread thread(&Client::AwaitServerResponse, this);
	thread.detach(); // Should last till timeout and then terminate.
}


void Client::AwaitServerResponse() {
	std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point now;
	std::chrono::duration<float, std::milli> duration;
	
	ENetEvent event;

	bool connected = false;
	bool timedout = false;

	while (!connected && !timedout) {
		if (enet_host_service(m_host, &event, 1000)) {
			if (event.type == ENET_EVENT_TYPE_CONNECT) {
				std::lock_guard<std::mutex> lock(m_connectionMutex);
				m_state = ConnectionState::CONNECTED;
				connected = true;
			}
			enet_packet_destroy(event.packet);
		}

		now = std::chrono::high_resolution_clock::now();
		duration = now - start;

		if (duration.count() > RESPONSE_WAIT_TIME) {
			timedout = true;
		}
	}
}


void Client::SendPacket(Packet::PacketBase packet) {
	ENetPacket* enetPacket = packet.ToENetPacket();
	enet_peer_send(m_server, packet.channel, enetPacket);
}


