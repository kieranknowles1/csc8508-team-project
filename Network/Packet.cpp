#include "Network.hpp"
#include "Packet.hpp"

using namespace Packet;

PacketBase PacketBase::FromENetPacket(ENetPacket* packet) {
	if (packet->dataLength < 1) return PacketBase(0, 0, 0);

	size_t offset = 0;

	Type type;
	memcpy(&type, packet->data, sizeof(type));
	offset += sizeof(type);

	unsigned short payloadSize;
	memcpy(&payloadSize, packet->data + offset, sizeof(payloadSize));
	offset += sizeof(payloadSize);

	char channel;
	memcpy(&channel, packet->data + offset, sizeof(channel));
	offset += sizeof(channel);

	std::shared_ptr<char[]> data = std::make_shared<char[]>(packet->dataLength - offset);
	memcpy(data.get(), packet->data + offset, packet->dataLength - offset);

	return PacketBase(type, payloadSize, channel, data);
}


ENetPacket* PacketBase::ToENetPacket() {
	char* packetData = new char[sizeof(type) + sizeof(size) + sizeof(channel) + (size * sizeof(char))];
	size_t offset = 0;

	memcpy(packetData, &type, sizeof(Type));
	offset += sizeof(Type);

	memcpy(packetData + offset, &size, sizeof(unsigned short));
	offset += sizeof(unsigned short);

	memcpy(packetData + offset, &channel, sizeof(char));
	offset += sizeof(char);

	memcpy(packetData + offset, data.get(), sizeof(char) * size);
	offset += sizeof(char) * size;

	ENetPacketFlag flags = ((type & 1) == 1) ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
	ENetPacket* packet = enet_packet_create(packetData, offset, flags);
	return packet;
}



