#pragma once

#include <./enet/enet.h>

#ifdef NETWORK_TEST
#include <iostream>

void DebugOut(const std::string& message) {
	std::cerr << "[DEBUG]: ";
	std::cerr << message << std::endl;
}

#endif


const unsigned int MAX_CLIENTS = 8;


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
	Network() { if (enet_initialize()) m_initialised = true; }
	~Network() { if (m_initialised) enet_deinitialize(); }

	/**
	 * @brief Call to determine if enet was successfully initialised.
	 * @return true if successful.
	 */
	bool IsInitialised() { return m_initialised; }

private:
	bool m_initialised = false;
};





