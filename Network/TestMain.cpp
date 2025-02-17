#include <stdexcept>
#include <iostream>

#include "Server.hpp"
#include "Packet.hpp"
#include "Client.hpp"


void TestPacketBuffer() {
	Packet::Packet packet1 = Packet::Packet(1, 0, 0);
	Packet::Packet packet2 = Packet::Packet(1, 0, 100);
	Packet::Packet packet3 = Packet::Packet(1, 0, 90);
	Packet::Packet packet4 = Packet::Packet(1, 0, 45);
	Packet::Packet packet5 = Packet::Packet(1, 0, 102);
	Packet::Packet packet6 = Packet::Packet(1, 0, 101);

	Packet::PacketBuffer buffer;
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

}



int main(int argc, char** argv) {
	TestPacketBuffer();

	return 0;
}
