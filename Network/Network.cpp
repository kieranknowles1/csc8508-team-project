#include "Network.hpp"

Network::Network() {
	if (enet_initialize() == 0) {
		m_initialised = true;
		m_state = NetworkState::INACTIVE;
	}
}


Network::~Network() {
	if (m_thread != nullptr) {
		m_thread->join();
		delete m_thread;
		m_thread = nullptr;
	}

	if (m_host) {
		enet_host_destroy(m_host);
		m_host = nullptr;
	}

	if (m_initialised) {
		enet_deinitialize();
		m_initialised = false;
	}
}
