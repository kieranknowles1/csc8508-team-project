#pragma once

#include <algorithm>
#include <array>
#include <thread>
#include <mutex>

#include <./enet/enet.h>

#include "Packet.hpp"


// The rate at which packets will be sent and received.
const float NETWORK_RATE = 1 / 60.0f; // Seconds

// How many packets to store before denying packets.
const int BUFFER_SIZE = 1024;

// Default binding port.
const int DEFAULT_PORT = 12345;


/**
 * @brief Character codes for console colors.
 */
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
	UNSEQUENCED,	// Used for packet data that cannot be lost but order is not important.
	FREQUENT,		// Used for packet data that can be lost and is frequently sent.
	CHANNEL_COUNT	// Used to count how many channels should be created.
};


/**
 * @brief Different states the network can be in.
 */
enum class NetworkState {
	CLOSED,
	OFF,
	ERRORED,
	ON
};


class Network {
public:
	Network(const ENetAddress* address, int maxConnections);
	~Network();

	/**
	 * @brief Start the thread for receiving and sending packets.
	 * 
	 * Starts a new thread. The thread handles sending and receiving of packets.
	 */
	void Start();
	void Stop();
	void Close();
	void ConnectTo(const ENetAddress* destination);

	void Send(Packet::Packet packet);
	Packet::Packet Fetch();

	NetworkState GetState() {
		std::lock_guard<std::mutex> lock(m_stateMut);
		return m_state;
	}

protected:
	void Run();				// Function the thread runs.
	void Tick(float dt);	// Tick the server (when to receive and send).
	void SendAll();			// Queue all the packets to be sent. Packets will send on next enet flush.

	void SetErrored();	// Set the server into an errored state.

private:
	bool ConnectPeer();
	void HandleIncomingPacket(ENetPacket* packet);
	void DisconnectPeer();

	std::thread m_networkThread;
	std::mutex m_stateMut;
	std::mutex m_sendMut;

	NetworkState m_state = NetworkState::CLOSED;

	Packet::PacketBuffer m_receiveBuffer = Packet::PacketBuffer(BUFFER_SIZE);
	std::vector<Packet::Packet> m_sendBuffer = std::vector<Packet::Packet>(BUFFER_SIZE);
	
	float m_elapsedTime = 0;
	float m_lastTick = 0;
	int m_lastMaxSequence = 0; // Each tick will drop optional packets that didn't make the first tick.

	int m_connections = 0;
	int m_maxConnections;

	ENetHost* m_host = nullptr;
};
