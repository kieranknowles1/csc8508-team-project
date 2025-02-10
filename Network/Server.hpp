#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <array>

#include <./enet/enet.h>

#include "Network.hpp"
#include "Packet.hpp"


enum class ServerState {
	STARTING,
	RUNNING,
	STOPPED,
	ERRORED
};


class Server : public Network {
public:
	Server() : m_hostName("localhost"), m_port(12345) { if (!IsInitialised()) m_state = ServerState::ERRORED; }
	Server(const std::string& hostName, const uint32_t port) : m_hostName(hostName), m_port(port) {};

	~Server();

	void Start();
	void Stop();

	std::string GetHostname() const { return m_hostName; }
	uint32_t GetPort() const { return m_port; }

protected:
	void Run();
	void Update();
	void AttemptConnection(ENetEvent& event);
	void RemoveConnection(ENetEvent& event);
	void Handle(Packet::PacketBase packet) {}

	ServerState m_state = ServerState::STOPPED;

private:
	std::string m_hostName;
	uint32_t m_port;

	std::thread* m_thread = nullptr;
	std::mutex m_stateMutex;

	ENetPeer* connections[MAX_CLIENTS];
	bool allowConnections = true;
	uint32_t m_numConnections = 0;
};

