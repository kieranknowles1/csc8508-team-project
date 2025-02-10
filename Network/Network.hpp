#pragma once

#include <./enet/enet.h>
//#include <./enet/time.h>

//#ifdef NETWORK_TEST
//#include <iostream>
//
//void DebugOut(const std::string& message) {
//	std::cerr << "[DEBUG]: ";
//	std::cerr << message << std::endl;
//}
//
//#endif


const unsigned int MAX_CLIENTS = 8;
const float EVENT_WAIT = 100; // ms

namespace ConsoleTextColor {
	inline const char* GREEN = "\x1b[32m";
	inline const char* YELLOW = "\x1b[33m";
	inline const char* DEFAULT = "\x1b[0m";
};

/**
 * @brief Channel enum class.
 * 
 * Defines the different channels that the network can use.
 * @warning Do not assign values to the channels.
 * 
 */
enum class Channel {
	RELIABLE = 0,	// MUST BE FIRST CHANNEL AS FIRST CHANNEL IS ALWAYS RELIABLE.
	FREQUENT,		// Used for packet data that can be lost and is frequently sent.
	CHANNEL_COUNT	// Used to count how many channels should be created.
};


/**
 * @brief Network class.
 * 
 * Basically handles initialisation and deinitialisation of enet.
 */
class Network {
public:
	Network() {
		if (enet_initialize() == 0) m_initialised = true;
	}
	~Network() {
		if (m_initialised) enet_deinitialize();
		if (m_host) enet_host_destroy(m_host);
		m_initialised = false;
	}

	/**
	 * @brief Call to determine if enet was successfully initialised.
	 * @return true if successful.
	 */
	bool IsInitialised() { return m_initialised; }

	/**
	 * @brief Wrapper function for enet_host_create().
	 * @see enet_host_create()
	 * @return true if host was successfully created.
	 */
	bool CreateHost(ENetAddress* address, int maxClients, int nChannels, int incBandwidth, int outBandwidth) {
		m_host = enet_host_create(address, maxClients, nChannels, incBandwidth, outBandwidth);
		return m_host != nullptr;
	}

	/**
	 * @brief Get an event from enet_host_service.
	 * @param waitTime	How long to wait for an event in ms
	 * @return The event from enet_host_service. Returns empty event if failed.
	 */
	ENetEvent GetEvent(float waitTime) {
		ENetEvent event;
		enet_host_service(m_host, &event, waitTime);
		return event;
	}

protected:
	ENetHost* m_host = nullptr;

private:
	bool m_initialised = false;
};





