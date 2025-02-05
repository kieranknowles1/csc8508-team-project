#pragma once

#ifdef NETWORK_TEST
#include <iostream>

void DebugOut(const std::string& message) {
	std::cerr << "[DEBUG]: ";
	std::cerr << message << std::endl;
}

#endif


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
	CHANNEL_COUNT
};
