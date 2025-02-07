#include <stdexcept>
#include <iostream>

#include "Server.hpp"
#include "Packet.hpp"

int main(int argc, char** argv) {
	Server server;

	std::shared_ptr<char[]> data = std::make_shared<char[]>(sizeof(char) * 5);
	data[0] = 'a';
	data[1] = 'b';
	data[2] = 'c';
	data[3] = 'd';
	data[4] = '\0';
	Packet::Type type = 4;
	unsigned short size = 5;
	char channel = 1;

	Packet::PacketBase packet(type, size, channel, data);
	ENetPacket* enetPacket = packet.ToENetPacket();
	Packet::PacketBase back = Packet::PacketBase::FromENetPacket(enetPacket);
	enet_packet_destroy(enetPacket);

	std::cout << packet.data << std::endl;
	std::cout << back.data << std::endl;
	

	server.Start();
	server.Stop();
	return 0;
}
