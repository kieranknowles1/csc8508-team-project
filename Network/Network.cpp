#include "Network.hpp"

Network::Network(int maxConnections) : m_maxConnections(maxConnections) {
	if (!enet_initialize()) {
		SetErrored();
		return;
	}

	m_host = enet_host_create(ENET_HOST_ANY, maxConnections, static_cast<int>(Channel::CHANNEL_COUNT), 0, 0);
	if (m_host == nullptr) {
		SetErrored();
		return;
	}

	std::lock_guard<std::mutex> lock(m_stateMut);
	m_state = NetworkState::OFF;
}


Network::~Network() {
	Stop();



	enet_deinitialize();
}


void Network::Start() {
	std::lock_guard<std::mutex> lock(m_stateMut);
	if (m_state != NetworkState::OFF) return;

	m_state = NetworkState::ON;
	m_networkThread = std::thread(Network::Run, this);
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







void Network::SetErrored() {
	std::lock_guard<std::mutex> lock(m_stateMut);
	m_state = NetworkState::ERRORED;
}
