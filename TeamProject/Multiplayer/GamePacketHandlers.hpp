#pragma once

#include "Multiplayer/GamePackets.hpp"

namespace Packet {

    class DeltaPacketHandler : public PacketHandler {
    public:
        DeltaPacketHandler() : PacketHandler(static_cast<Type>(PacketType::DELTA)) {}

        void Handle(const std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> Translate(const ENetEvent* event) const override;
        ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
    };


    class LaserPacketHandler : public PacketHandler {
    public:
        LaserPacketHandler() : PacketHandler(static_cast<Type>(PacketType::LASER)) {}

        void Handle(const std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> Translate(const ENetEvent* event) const override;
        ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
    };


    class PositionPacketHandler : public PacketHandler {
    public:
        PositionPacketHandler() : PacketHandler(static_cast<Type>(PacketType::POSITION)) {}

        void Handle(const std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> Translate(const ENetEvent* event) const override;
        ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
    };


    class PlayerStateChangePacketHandler : public PacketHandler {
    public:
        PlayerStateChangePacketHandler() : PacketHandler(static_cast<Type>(PacketType::PLAYER_STATE_CHANGE)) {}

        void Handle(const std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> Translate(const ENetEvent* event) const override;
        ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
    };


    class ObjectChangeGravityPacketHandler : public PacketHandler {
    public:
        ObjectChangeGravityPacketHandler() : PacketHandler(static_cast<Type>(PacketType::OBJECT_CHANGE_GRAVITY)) {}

        void Handle(const std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> Translate(const ENetEvent* event) const override;
        ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
    };


    class StartGamePacketHandler : public PacketHandler {
    public:
        StartGamePacketHandler() : PacketHandler(static_cast<Type>(PacketType::START_GAME)) {}

        void Handle(const std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> Translate(const ENetEvent* event) const override;
        ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
    };


    class UserInfoPacketHandler : public PacketHandler {
    public:
        UserInfoPacketHandler() : PacketHandler(static_cast<Type>(PacketType::USER_INFO)) {}

        void Handle(const std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> Translate(const ENetEvent* event) const override;
        ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
    };


    class DamagePacketHandler : public PacketHandler {
    public:
        DamagePacketHandler() : PacketHandler(static_cast<Type>(PacketType::DAMAGE)) {}

        void Handle(const std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> Translate(const ENetEvent* event) const override;
        ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
    }; 


    class PingPacketHandler : public PacketHandler {
    public:
        PingPacketHandler() : PacketHandler(static_cast<Type>(PacketType::PING)) {}

        void Handle(const std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> Translate(const ENetEvent* event) const override;
        ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
    }; 


    class PongPacketHandler : public PacketHandler {
    public:
        PongPacketHandler() : PacketHandler(static_cast<Type>(PacketType::PONG)) {}

        void Handle(const std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> Translate(const ENetEvent* event) const override;
        ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const override;
    };
}
