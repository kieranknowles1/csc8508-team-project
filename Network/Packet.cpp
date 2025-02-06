#include "Network.hpp"
#include "Packet.hpp"

using namespace Packet;

PacketBase PacketBase::FromENetEvent(ENetEvent* event) {
	if (event->packet->dataLength < 1) return PacketBase(0, 0, 0);

	size_t offset = 0;

	Type type;
	memcpy(&type, event->packet->data, sizeof(type));
	offset += sizeof(type);

	unsigned short payloadSize;
	memcpy(&payloadSize, event->packet->data + offset, sizeof(payloadSize));
	offset += sizeof(payloadSize);

	char channel;
	memcpy(&channel, event->packet->data + offset, sizeof(channel));
	offset += sizeof(channel);

	std::shared_ptr<char[]> data = std::make_shared<char[]>(event->packet->dataLength - offset);
	memcpy(data.get(), event->packet->data + offset, event->packet->dataLength - offset);

	return PacketBase(type, payloadSize, channel, data);
}


ENetPacket* PacketBase::ToENetPacket() {
	char* data = new char[sizeof(type) + sizeof(size) + sizeof(channel) + (size * sizeof(char))];
	size_t offset = 0;

	memcpy(data, this, sizeof(Type));
	offset += sizeof(Type);

	memcpy(data + offset, this + offset, sizeof(unsigned short));
	offset += sizeof(unsigned short);

	memcpy(data + offset, this + offset, sizeof(char));
	offset += sizeof(char);

	memcpy(data + offset, this + offset, sizeof(char) * size);
	offset += sizeof(char) * size;

	ENetPacketFlag flags = ((type & 1) == 1) ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
	ENetPacket* packet = enet_packet_create(data, offset, flags);
	return packet;
}



