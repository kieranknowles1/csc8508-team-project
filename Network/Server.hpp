#include <string>

#include <NetworkBase.h>

template <typename Packet>
class PacketHandler {
public:
	PacketHandler();
	~PacketHandler();

private:
	

};

class Server {
public:
	Server() : m_hostName("localhost"), m_port(12345) {}
	Server(std::string hostName, int port) : m_hostName(hostName), m_port(port) {};




private:
	std::string m_hostName;
	int m_port;

};
