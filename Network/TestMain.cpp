#include <stdexcept>
#include <iostream>
#include <chrono>

#include "Network.hpp"

using namespace std::chrono_literals;

void TestPacketBuffer() {
	Packet::Packet packet1 = Packet::Packet(1, 0, 0);
	Packet::Packet packet2 = Packet::Packet(1, 0, 100);
	Packet::Packet packet3 = Packet::Packet(1, 0, 90);
	Packet::Packet packet4 = Packet::Packet(1, 0, 45);
	Packet::Packet packet5 = Packet::Packet(1, 0, 102);
	Packet::Packet packet6 = Packet::Packet(1, 0, 101);

	Packet::PacketBuffer buffer(16);
	buffer.Insert(packet1);
	buffer.Insert(packet2);
	buffer.Insert(packet3);
	buffer.Insert(packet4);
	buffer.Insert(packet5);
	buffer.Insert(packet6);

	while (!buffer.IsEmpty()) {
		Packet::Packet packet = buffer.Pop();
		std::cout << ConsoleTextColor::YELLOW << "Sequence Number: " << ConsoleTextColor::DEFAULT << packet.GetSequenceNumber() << std::endl;
	}

	ENetAddress destination;
	enet_address_set_host(&destination, "10.70.33.113");
	destination.port = DEFAULT_PORT;

	std::vector<std::unique_ptr<Network>> clients;

	ENetAddress serverAddr(ENET_HOST_ANY, DEFAULT_PORT);
	Network server = Network(&serverAddr, 1);
	server.Start();

	std::cout << "Starting clients.\n";
	for (int i = 0; i < 1; ++i) {
		clients.push_back(std::make_unique<Network>(nullptr, 1));
		if (clients[i].get()->GetState() == NetworkState::ERRORED) {
			std::cout << "Client: " << i << " is broken.\n";
		}
		else {
			clients[i]->Start();
			clients[i]->ConnectTo(&destination);
		}
	}

	std::this_thread::sleep_for(1000ms);

	server.Close();
	for (int i = 0; i < 1; ++i) {
		clients[i].get()->Close();
		std::cout << "joined.\n";
	}
}


int main(int argc, char** argv) {
	TestPacketBuffer();

	return 0;
}
