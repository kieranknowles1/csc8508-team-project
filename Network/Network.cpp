#include <chrono>

#include "Network.hpp"

Network::Network(const ENetAddress* address, int maxConnections) : m_maxConnections(maxConnections) {
	if (!enet_initialize()) {
		SetErrored();
		return;
	}

	m_host = enet_host_create(address, maxConnections, static_cast<int>(Channel::CHANNEL_COUNT), 0, 0);
	if (m_host == nullptr) {
		SetErrored();
		return;
	}

	std::lock_guard<std::mutex> lock(m_stateMut);
	m_state = NetworkState::OFF;
}


Network::~Network() {
	Stop();
	Close();
}


void Network::Start() {
	std::lock_guard<std::mutex> lock(m_stateMut);
	if (m_state != NetworkState::OFF) return;

	m_state = NetworkState::ON;
	m_networkThread = std::thread(&Network::Run, this);
}


void Network::Stop() {
	std::lock_guard<std::mutex> lock(m_stateMut);
	if (!(m_state == NetworkState::ON)) return;

	m_state = NetworkState::OFF;
	m_networkThread.join();
}


void Network::Close() {
	Stop();

	for (int peer = 0; peer < m_host->peerCount; ++peer) {
		enet_peer_disconnect(&(m_host->peers[peer]), 0);
	}
	m_connections = 0;

	enet_deinitialize();
}


void Network::Send(Packet::Packet packet) {
	std::lock_guard<std::mutex> lock(m_sendMut);
	m_sendBuffer.push_back(packet);
}


Packet::Packet Network::Fetch() {
	return m_receiveBuffer.Pop();
}


void Network::Run() {
	auto last = std::chrono::high_resolution_clock::now();
	auto now = last;
	std::chrono::duration<float> dt;

	bool running = true;

	while (running) {
		if (GetState() != NetworkState::ON) {
			running = false;
			return;
		}

		now = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration_cast<std::chrono::seconds>(last - now);

		Tick(dt.count());
	}
}


void Network::Tick(float dt) {
	m_elapsedTime += dt;

	while (m_elapsedTime - m_lastTick >= NETWORK_RATE) {
		m_lastTick += NETWORK_RATE;

		SendAll();

		ENetEvent event;
		while (enet_host_service(m_host, &event, 0) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				if (!ConnectPeer()) enet_peer_disconnect(event.peer, 0);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				DisconnectPeer();
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				HandleIncomingPacket(event.packet);
				break;
			}
			enet_packet_destroy(event.packet);
		}
	}
}


void Network::SendAll() {
	auto current = m_sendBuffer.begin();
	auto end = m_sendBuffer.end();

	Packet::PacketRegister* packetRegister = Packet::PacketRegister::GetRegister();

	do {
		Packet::PacketHandler* handler = packetRegister->GetHandler(current->GetType());
		ENetPacket* packet = handler->ToENetPacket(*current);
		enet_host_broadcast(m_host, current->GetChannel(), packet);
	} while (current != end);
}


void Network::SetErrored() {
	std::lock_guard<std::mutex> lock(m_stateMut);
	m_state = NetworkState::ERRORED;
}


bool Network::ConnectPeer() {
	if (m_connections < m_maxConnections) {
		m_connections++;
		return true;
	}
	return false;
}


void Network::HandleIncomingPacket(ENetPacket* packet) {
	Packet::Type packetType;
	memcpy(&packetType, packet->data, sizeof(Packet::Type));
		
	Packet::PacketRegister* packetRegister = Packet::PacketRegister::GetRegister();
	Packet::PacketHandler* packetHandler = packetRegister->GetHandler(packetType);
	Packet::Packet translated = packetHandler->Translate(packet);

	m_receiveBuffer.Insert(translated);
}


void Network::DisconnectPeer() {
	m_connections--;
	if (m_connections < 0) SetErrored();
}
