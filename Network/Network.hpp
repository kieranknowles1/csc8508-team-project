#pragma once

#include <algorithm>
#include <array>
#include <thread>
#include <mutex>
#include <functional>

#include <./enet/enet.h>

#include "Packet.hpp"


// The rate at which packets will be sent and received.
const float NETWORK_RATE = 1 / 60.0f; // Seconds

// How many packets to store before denying packets.
const int BUFFER_SIZE = 1024;

// Default binding port.
const int DEFAULT_PORT = 49834;


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

	/**
	 * @brief Stop and join the execution thread.
	 * 
	 * Sets network state to OFF
	 */
	void Stop();

	/**
	 * @brief Closes the host.
	 * 
	 * Cleans up enet objects. Calls Stop() first.
	 */
	void Close();

	/**
	 * @brief Connect to another ENetHost.
	 * @param destination The address of the endpoint to connect to.
	 */
	void ConnectTo(const ENetAddress* destination);

	/**
	 * @brief Queue a Packet to be send next tick via Broadcasting.
	 * Is threadsafe.
	 * @param packet The packet to send.
	 */
	void Send(std::shared_ptr<Packet::Packet> packet);

	/**
	 * @brief Assign which function to call upon receiving a connect packet.
	 * 
	 * Passes ENetPeer* incase the callback function requires it.
	 * 
	 * @param callback The function to call.
	 */
	inline void SetConnectCallback(std::function<void(ENetPeer*)> callback) { m_connectCallback = callback; }

	/**
	 * @brief Fetch a packet from the buffer.
	 * 
	 * Fetch from the buffer. The buffer is updated every tick. Is threadsafe.
	 * 
	 * @return The next packet in the buffer. Empty packet if no packet exists.
	 */
	std::shared_ptr<Packet::Packet> Fetch();

	/**
	 * @brief Get the current state of the network.
	 * Is Threadsafe.
	 * @return 
	 */
	NetworkState GetState() {
		std::lock_guard<std::mutex> lock(m_stateMut);
		return m_state;
	}

	/**
	 * @brief Get the number of external connections.
	 * @return int representing the number of external connections.
	 */
	int GetConnectionCount() const { return m_connections - 1; }


protected:
	/**
	 * @brief The entry point for the thread for sending and receiving packets.
	 */
	void Run();

	/**
	 * @brief Step forward by dt.
	 * 
	 * If enough time has elapsed the server will fetch and send packets based
	 * on the NETWORK_RATE.
	 */
	void Tick(float dt);

	/**
	 * @brief Sends all queued packets.
	 * Broadcasts to all connections.
	 */
	void SendAll();

	/**
	 * @brief Set the network into an errored state.
	 */
	void SetErrored();

private:
	bool ConnectPeer();
	void HandleIncomingPacket(ENetPacket* packet);
	void DisconnectPeer();

	std::thread m_networkThread;
	std::mutex m_stateMut;
	std::mutex m_sendMut;

	NetworkState m_state = NetworkState::CLOSED;

	Packet::PacketBuffer m_receiveBuffer = Packet::PacketBuffer(BUFFER_SIZE);
	std::vector<std::shared_ptr<Packet::Packet>> m_sendBuffer = std::vector<std::shared_ptr<Packet::Packet>>(BUFFER_SIZE);
	int m_numPackets = 0;
	
	float m_elapsedTime = 0;
	float m_lastTick = 0;
	int m_lastMaxSequence = 0; // Each tick will drop optional packets that didn't make the first tick.

	int m_connections = 1;
	int m_maxConnections;

	ENetHost* m_host = nullptr;
	std::function<void(ENetPeer*)> m_connectCallback;
};
