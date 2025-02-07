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

	ENetAddress clientAddress = { ENET_HOST_ANY, 12346 };
	ENetAddress serverAddress = { 0, 12345 };
	enet_address_set_host(&serverAddress, "192.168.0.15");

	ENetHost* clientHost = enet_host_create(&clientAddress, 1, 2, 0, 0);

	ENetPeer* serverConn = enet_host_connect(clientHost, &serverAddress, 2, 0);

	// Wait for server response.
	ENetEvent event;
	bool connected = false;
	while (enet_host_service(clientHost, &event, 1000) && !connected) {
		if (event.type == ENET_EVENT_TYPE_CONNECT) {
			std::cout << ConsoleTextColor::GREEN << "[Client] Server found!\n";
			connected = true;
			continue;
		}
		std::cout << ConsoleTextColor::GREEN << "[Client] No response from server. Trying again.\n";
	}

	char hostName[16];
	enet_address_get_host_ip(&serverConn->address, hostName, 16);
	std::cout << ConsoleTextColor::GREEN << "[Client] Connected to " << hostName << ":" << serverConn->address.port << std::endl;

	const char* discoverData = "DISCOVER";

	// Create sample packet.
	Packet::PacketBase hello;
	hello.type = Packet::DISCOVER;
	hello.size = strlen(discoverData) + 1;
	hello.channel = 0;
	hello.data = std::make_shared<char[]>(strlen(discoverData) + 1);
	memcpy(hello.data.get(), discoverData, hello.size);

	ENetPacket* discoverPacket = hello.ToENetPacket();

	enet_peer_send(serverConn, 0, discoverPacket);
	enet_host_flush(clientHost);


	server.Stop();

	std::cout << ConsoleTextColor::DEFAULT;
	return 0;
}
