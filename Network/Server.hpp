#pragma once

#include <string>
#include <thread>
#include <mutex>

#include <./enet/enet.h>

#include "Network.hpp"







template <typename Packet>
class PacketHandler {
public:
	PacketHandler();
	~PacketHandler();

private:
	

};


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

	void Start(bool threaded = false);
	void Stop();

	std::string GetHostname() const { return m_hostName; }
	uint32_t GetPort() const { return m_port; }




protected:
	void CreateHost();

	ServerState m_state = ServerState::STOPPED;

private:
	std::string m_hostName;
	uint32_t m_port;
	ENetAddress address;

	std::thread* m_thread = nullptr;
	std::mutex m_stateMutex;

	ENetHost* m_host;
};

