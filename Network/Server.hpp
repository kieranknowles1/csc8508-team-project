#pragma once

#include <string>
#include <thread>
#include <mutex>

#include <NetworkBase.h>


#ifdef NETWORK_TEST
#include <iostream>

void DebugOut(const std::string& message) {
	std::cerr << "[DEBUG]: ";
	std::cerr << message << std::endl;
}

#endif


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


class Server {
public:
	Server() : m_hostName("localhost"), m_port(12345) {}
	Server(const std::string& hostName, const uint32_t port) : m_hostName(hostName), m_port(port) {};

	~Server();

	void Start(bool threaded = false);
	void Stop();

	std::string GetHostname() const { return m_hostName; }
	uint32_t GetPort() const { return m_port; }


protected:
	ServerState m_state = ServerState::STOPPED;

private:
	std::string m_hostName;
	uint32_t m_port;

	std::thread* m_thread = nullptr;
	std::mutex state_mutex;
};

