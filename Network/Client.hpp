#pragma once

#include <mutex>

#include "Packet.hpp"
#include "Network.hpp"


enum class ConnectionState {
	CONNECTED,
	WAITING,
	DISCONNECTED
};


inline const float RESPONSE_WAIT_TIME = 10000; // 10s


class Client : public Network {
public:
	Client();
	~Client();

	/**
	 * @brief Connect to a server.
	 * @param address The address of the server.
	 * 
	 * Attempts to connect to a server. Sets the state to WAITING.
	 * Spawns a separate thread that lasts until the time elapsed exceeds
	 * RESPONSE_WAIT_TIME or the client receives a connection response from the
	 * server.
	 */
	void ConnectTo(ENetAddress& address);

	/**
	 * @brief Get the current connection state of the client.
	 * @return The current ConnectionState
	 */
	ConnectionState GetState() {
		std::lock_guard<std::mutex> lock(m_connectionMutex);
		return m_state;
	}

	/**
	 * @brief Sends the packet to the server.
	 * @param packet The packet to be transmitted.
	 */
	void QueuePacket(Packet::PacketBase packet);


	/**
	 * @brief Sends all packets not currently sent. Lazy execution.
	 */
	void Flush() { enet_host_flush(m_host); }

private:
	/**
	 * @brief Waits for a response from the server.
	 * 
	 * Waits until @see RESPONSE_WAIT_TIME is exceeded.
	 * 
	 * If response is received, state is changed to CONNECTED.
	 * If timed out, state is changed to DISCONNECTED.
	 */
	void AwaitServerResponse();

	std::mutex m_connectionMutex;
	ConnectionState m_state;
	ENetPeer* m_server;
};
