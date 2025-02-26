#pragma once

#include "GamePackets.hpp"

namespace Packet {

	class DeltaPacketHandler : public PacketHandler {
	public:
		DeltaPacketHandler() : PacketHandler(static_cast<Type>(PacketType::DELTA)) {}

		void Handle(Packet packet) override;
		Packet Translate(ENetPacket* packet) override;
		ENetPacket* ToENetPacket(const Packet& packet) override;
	};


	class PositionPacketHandler : public PacketHandler {
	public:
		PositionPacketHandler() : PacketHandler(static_cast<Type>(PacketType::POSITION)) {}

		void Handle(Packet packet) override;
		Packet Translate(ENetPacket* packet) override;
		ENetPacket* ToENetPacket(const Packet& packet) override;
	};


	class PlayerStateChangePacketHandler : public PacketHandler {
	public:
		PlayerStateChangePacketHandler() : PacketHandler(static_cast<Type>(PacketType::PLAYER_STATE_CHANGE)) {}

		void Handle(Packet packet) override;
		Packet Translate(ENetPacket* packet) override;
		ENetPacket* ToENetPacket(const Packet& packet) override;
	};


	class ObjectChangeGravityPacketHandler : public PacketHandler {
	public:
		ObjectChangeGravityPacketHandler() : PacketHandler(static_cast<Type>(PacketType::OBJECT_CHANGE_GRAVITY)) {}

		void Handle(Packet packet) override;
		Packet Translate(ENetPacket* packet) override;
		ENetPacket* ToENetPacket(const Packet& packet) override;
	};
}
