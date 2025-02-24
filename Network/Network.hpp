#pragma once

#include <algorithm>
#include <array>
#include <thread>
#include <mutex>

#include <./enet/enet.h>

#include "Packet.hpp"


// The rate at which packets will be sent and received.
const float NETWORK_RATE = 1 / 60.0f; // Seconds

// How long the server will wait for packets sent unreliably (in ticks).
const float NETWORK_DELAY = 2; // Ticks (1 tick = 1 NETWORK_RATE)

// How many packets to store before denying packets.
const int BUFFER_SIZE = 1024;

// How long to wait for a packet (enet_host_service).
const float EVENT_WAIT = NETWORK_RATE * 0.5f; // Seconds


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


/**
 * @brief Network class.
 * 
 * Basically handles initialisation and deinitialisation of enet.
 */
//class NetworkA {
//public:
//	NetworkA();
//	~NetworkA();
//
//	/**
//	 * @brief Call to determine if enet was successfully initialised.
//	 * @return true if successful.
//	 */
//	inline bool IsInitialised() { return m_initialised; }
//
//	/**
//	 * @brief Wrapper function for enet_host_create().
//	 * @see enet_host_create()
//	 * @return true if host was successfully created.
//	 */
//	inline bool CreateHost(ENetAddress* address, int maxClients, int nChannels, int incBandwidth, int outBandwidth) {
//		m_host = enet_host_create(address, maxClients, nChannels, incBandwidth, outBandwidth);
//		return m_host != nullptr;
//	}
//
//	/**
//	 * @brief Get an event from enet_host_service.
//	 * @return The event from enet_host_service. Returns empty event if failed.
//	 */
//	inline ENetEvent GetEvent() {
//		ENetEvent event;
//		enet_host_service(m_host, &event, EVENT_WAIT);
//		return event;
//	}
//
//	void Stop() {
//		m_stateMutex.lock();
//		m_state = NetworkState::INACTIVE;
//		m_stateMutex.unlock();
//
//		m_thread->join();
//	}
//
//	void Update();
//
//	void Start();
//
//	virtual void Handle(ENetPacket* packet);
//
//protected:
//	ENetHost* m_host = nullptr;
//	NetworkState m_state = NetworkState::ERRORED;
//
//	std::mutex m_stateMutex;
//	std::thread* m_thread = nullptr;
//
//private:
//	bool m_initialised = false;
//};



class Network {
public:
	Network(int maxConnections);
	~Network();

	void Start();
	void Stop();
	void Close();

	void Send(Packet::Packet packet);
	Packet::Packet Fetch();

protected:
	void Run();				// Function the thread runs.
	void Tick(float dt);	// Tick the server (when to receive and send).

	void SetErrored();	// Set the server into an errored state.

private:
	void SendAll();

	std::thread m_networkThread;
	std::mutex m_stateMut;
	NetworkState m_state = NetworkState::CLOSED;

	Packet::PacketBuffer m_receiveBuffer = Packet::PacketBuffer(BUFFER_SIZE);
	std::vector<Packet::Packet> m_sendBuffer = std::vector<Packet::Packet>(BUFFER_SIZE);
	
	float elapsedTime = 0;

	int m_connections = 0;
	int m_maxConnections;

	ENetHost* m_host = nullptr;
};
