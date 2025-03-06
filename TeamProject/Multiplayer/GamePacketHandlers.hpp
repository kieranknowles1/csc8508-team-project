#pragma once

#include "Multiplayer/GamePackets.hpp"

namespace Packet {

	class DeltaPacketHandler : public PacketHandler {
	public:
		DeltaPacketHandler() : PacketHandler(static_cast<Type>(PacketType::DELTA)) {}

		void Handle(const std::shared_ptr<Packet> packet) override;
		std::shared_ptr<Packet> Translate(const ENetPacket* packet) const override;
		ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
	};


	class PositionPacketHandler : public PacketHandler {
	public:
		PositionPacketHandler() : PacketHandler(static_cast<Type>(PacketType::POSITION)) {}

		void Handle(const std::shared_ptr<Packet> packet) override;
		std::shared_ptr<Packet> Translate(const ENetPacket* packet) const override;
		ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
	};


	class PlayerStateChangePacketHandler : public PacketHandler {
	public:
		PlayerStateChangePacketHandler() : PacketHandler(static_cast<Type>(PacketType::PLAYER_STATE_CHANGE)) {}

		void Handle(const std::shared_ptr<Packet> packet) override;
		std::shared_ptr<Packet> Translate(const ENetPacket* packet) const override;
		ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
	};


	class ObjectChangeGravityPacketHandler : public PacketHandler {
	public:
		ObjectChangeGravityPacketHandler() : PacketHandler(static_cast<Type>(PacketType::OBJECT_CHANGE_GRAVITY)) {}

		void Handle(const std::shared_ptr<Packet> packet) override;
		std::shared_ptr<Packet> Translate(const ENetPacket* packet) const override;
		ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
	};


	class StartGamePacketHandler : public PacketHandler {
	public:
		StartGamePacketHandler() : PacketHandler(static_cast<Type>(PacketType::START_GAME)) {}

		void Handle(const std::shared_ptr<Packet> packet) override;
		std::shared_ptr<Packet> Translate(const ENetPacket* packet) const override;
		ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
	};


	class UserInformationPacketHandler : public PacketHandler {
	public:
		UserInformationPacketHandler() : PacketHandler(static_cast<Type>(PacketType::USER_INFORMATION)) {}

		void Handle(const std::shared_ptr<Packet> packet) override;
		std::shared_ptr<Packet> Translate(const ENetPacket* packet) const override;
		ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
	};
}
