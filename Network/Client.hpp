//#pragma once
//
//#include <mutex>
//
//#include "Packet.hpp"
//#include "Network.hpp"
//
//
//enum class ConnectionState {
//	CONNECTED,
//	WAITING,
//	DISCONNECTED
//};
//
//
//class Client : public Network {
//public:
//	Client();
//	~Client();
//
//	/**
//	 * @brief Connect to a server.
//	 * @param address The address of the server.
//	 * 
//	 * Attempts to connect to a server.
//	 * Spawns a new thread to receive responses from the server. If no response is
//	 * received, client disconnects.
//	 */
//	void ConnectTo(ENetAddress& address);
//
//	/**
//	 * @brief Get the current connection state of the client.
//	 * @return The current ConnectionState
//	 */
//	ConnectionState GetState() {
//		std::lock_guard<std::mutex> lock(m_connectionMutex);
//		return m_state;
//	}
//
//	/**
//	 * @brief Sends the packet to the server.
//	 * @param packet The packet to be transmitted.
//	 */
//	void QueuePacket() {};
//
//
//	/**
//	 * @brief Sends all packets not currently sent. Lazy execution.
//	 */
//	void Flush() { enet_host_flush(m_host); }
//
//
//	/**
//	 * @brief Receive an incoming packet.
//	 */
//	void Update();
//
//private:
//
//	std::mutex m_connectionMutex;
//	std::thread* m_thread;
//
//	ConnectionState m_state;
//	ENetPeer* m_server;
//};
