#include <stdexcept>
#include <iostream>

#include "Server.hpp"
#include "Packet.hpp"
#include "Client.hpp"

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

	ENetAddress serverAddress = { 0, 12345 };
	enet_address_set_host(&serverAddress, "10.70.33.113");

	Client client;
	client.ConnectTo(serverAddress);

	const char* hello = "HELLO";

	Packet::PacketBase discover;
	discover.type = Packet::DISCOVER;
	discover.size = strlen(hello) + 1;
	discover.channel = 0;
	discover.data = std::make_shared<char[]>(discover.size);
	memcpy(discover.data.get(), hello, discover.size);


	while (client.GetState() != ConnectionState::CONNECTED) {
		std::cout << ConsoleTextColor::DEFAULT << "Waiting...\n";
	}
	std::cout << ConsoleTextColor::GREEN << "[Client] Connected to server.\n";


	client.QueuePacket(discover);
	client.Flush();

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	server.Stop();

	std::cout << ConsoleTextColor::DEFAULT;
	return 0;
}
