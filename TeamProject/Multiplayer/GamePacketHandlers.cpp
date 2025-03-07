#include "TutorialGame.h"
#include "GameObject.h"

#include "Multiplayer/GamePacketHandlers.hpp"

namespace Packet {

#pragma region DeltaPacketHandler
	void DeltaPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
		const DeltaPacket* deltaPacket = std::static_pointer_cast<DeltaPacket>(packet).get();
		GameObject* object = GameObject::GetGameObjectByID(deltaPacket->GetTargetID());

		object->GetPhysicsObject()->GetRigidBody()->setLinearVelocity(deltaPacket->GetLinearVelocity());
		object->GetPhysicsObject()->GetRigidBody()->setAngularVelocity(deltaPacket->GetAngularVelocity());
	}
	
	std::shared_ptr<Packet> DeltaPacketHandler::Translate(const ENetEvent* event) const {
		ENetPacket* packet = event->packet;
		Type type;
		uint8_t channel;
		uint32_t sequenceNumber;
		
		int objectID;
		btVector3 linearVelocity;
		btVector3 angularVelocity;
		size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

		GetBaseData(packet, &type, &channel, &sequenceNumber);

		memcpy(&objectID, packet->data + offset, sizeof(int));
		offset += sizeof(int);

		memcpy(&linearVelocity, packet->data + offset, sizeof(btVector3));
		offset += sizeof(btVector3);

		memcpy(&angularVelocity, packet->data + offset, sizeof(btVector3));
		offset += sizeof(btVector3);

		return std::make_shared<DeltaPacket>(objectID, linearVelocity, angularVelocity, sequenceNumber);
	}
	
	ENetPacket* DeltaPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		char* buffer = new char[
			sizeof(Type)
			+ sizeof(uint8_t)
			+ sizeof(uint32_t)
			+ sizeof(int)
			+ sizeof(btVector3)
			+ sizeof(btVector3)
		];

		DeltaPacket deltaPacket = (*static_cast<DeltaPacket*>(packet.get()));
		size_t offset = 0;

		Type type = deltaPacket.GetType();
		memcpy(buffer, &type, sizeof(Type));
		offset = offset + sizeof(Type);

		uint8_t channel = deltaPacket.GetChannel();
		memcpy(buffer + offset, &channel, sizeof(uint8_t));
		offset = offset + sizeof(uint8_t);

		uint32_t sequenceNumber = deltaPacket.GetSequenceNumber();
		memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
		offset = offset + sizeof(uint32_t);

		int objectID = deltaPacket.GetTargetID();
		memcpy(buffer + offset, &objectID, sizeof(int));
		offset = offset + sizeof(int);

		btVector3 linearVelocity = deltaPacket.GetLinearVelocity();
		memcpy(buffer + offset, &linearVelocity, sizeof(btVector3));
		offset = offset + sizeof(btVector3);

		btVector3 angularVelocity = deltaPacket.GetAngularVelocity();
		memcpy(buffer + offset, &angularVelocity, sizeof(btVector3));
		offset = offset + sizeof(btVector3);

		int packetFlags = 0;
		if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
		else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

		ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
		return enetPacket;
	}
#pragma endregion DeltaPacketHandler


#pragma region PositonPacketHandler
	void PositionPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
		const PositionPacket* positionPacket = std::static_pointer_cast<PositionPacket>(packet).get();
		GameObject* targetObject = GameObject::GetGameObjectByID(positionPacket->GetTargetID());
		btRigidBody* body = targetObject->GetPhysicsObject()->GetRigidBody();

		body->getWorldTransform().setOrigin(positionPacket->GetPosition());
		body->getWorldTransform().setRotation(positionPacket->GetOrientation());
	}

	std::shared_ptr<Packet> PositionPacketHandler::Translate(const ENetEvent* event) const {
		ENetPacket* packet = event->packet;
		Type type;
		uint8_t channel;
		uint32_t sequenceNumber;
		
		int objectID;
		btVector3 position;
		btQuaternion orientation;
		size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

		GetBaseData(packet, &type, &channel, &sequenceNumber);

		memcpy(&objectID, packet->data + offset, sizeof(int));
		offset += sizeof(int);

		memcpy(&position, packet->data + offset, sizeof(btVector3));
		offset += sizeof(btVector3);

		memcpy(&orientation, packet->data + offset, sizeof(btQuaternion));
		offset += sizeof(btQuaternion);

		return std::make_shared<PositionPacket>(objectID, position, orientation, sequenceNumber);
	}

	ENetPacket* PositionPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		char* buffer = new char[
			sizeof(Type)
			+ sizeof(uint8_t)
			+ sizeof(uint32_t)
			+ sizeof(int)
			+ sizeof(btVector3)
			+ sizeof(btQuaternion)
		];

		PositionPacket positionPacket = (*static_cast<PositionPacket*>(packet.get()));
		size_t offset = 0;

		Type type = positionPacket.GetType();
		memcpy(buffer, &type, sizeof(Type));
		offset = offset + sizeof(Type);

		uint8_t channel = positionPacket.GetChannel();
		memcpy(buffer + offset, &channel, sizeof(uint8_t));
		offset = offset + sizeof(uint8_t);

		uint32_t sequenceNumber = positionPacket.GetSequenceNumber();
		memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
		offset = offset + sizeof(uint32_t);

		int objectID = positionPacket.GetTargetID();
		memcpy(buffer + offset, &objectID, sizeof(int));
		offset = offset + sizeof(int);

		btVector3 position = positionPacket.GetPosition();
		memcpy(buffer + offset, &position, sizeof(btVector3));
		offset = offset + sizeof(btVector3);

		btQuaternion orientation = positionPacket.GetOrientation();
		memcpy(buffer + offset, &orientation, sizeof(btQuaternion));
		offset = offset + sizeof(btQuaternion);

		int packetFlags = 0;
		if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
		else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

		ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
		return enetPacket;
	}
#pragma endregion PositonPacketHandler


#pragma region PlayerStateChangePacketHandler
	void PlayerStateChangePacketHandler::Handle(const std::shared_ptr<Packet> packet) {

	}

	std::shared_ptr<Packet> PlayerStateChangePacketHandler::Translate(const ENetEvent* event) const {
		return std::make_shared<Packet>();
	}

	ENetPacket* PlayerStateChangePacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		return nullptr;
	}
#pragma endregion PlayerStateChangePacketHandler


#pragma region ObjectChangeGravityPacketHandler
	void ObjectChangeGravityPacketHandler::Handle(const std::shared_ptr<Packet> packet) {

	}

	std::shared_ptr<Packet> ObjectChangeGravityPacketHandler::Translate(const ENetEvent* event) const {
		return std::make_shared<Packet>();
	}

	ENetPacket* ObjectChangeGravityPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		return nullptr;
	}
#pragma endregion ObjectChangeGravityPacketHandler


#pragma region StartGamePacketHandler
	void StartGamePacketHandler::Handle(const std::shared_ptr<Packet> packet) {
	
	}

	std::shared_ptr<Packet> StartGamePacketHandler::Translate(const ENetEvent* event) const {
		return std::make_shared<StartGamePacket>();
	}

	ENetPacket* StartGamePacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		return nullptr;
	}
#pragma endregion StartGamePacketHandler


#pragma region AssignHostPacketHandler
	void AssignHostPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
		const AssignHostPacket* hostPacket = std::static_pointer_cast<AssignHostPacket>(packet).get();
		std::optional<Lobby>& lobby = TutorialGame::GetLobby();
		if (lobby.has_value()) lobby->SetHost(hostPacket->GetHostID());
	}

	std::shared_ptr<Packet> AssignHostPacketHandler::Translate(const ENetEvent* event) const {
		ENetPacket* packet = event->packet;
		Type type;
		uint8_t channel;
		uint32_t sequenceNumber;
		
		int hostID;
		size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

		GetBaseData(packet, &type, &channel, &sequenceNumber);

		memcpy(&hostID, packet->data + offset, sizeof(int));
		offset += sizeof(int);

		return std::make_shared<AssignHostPacket>(hostID, event->peer);
	}

	ENetPacket* AssignHostPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		char* buffer = new char[
			sizeof(Type),
			sizeof(uint8_t),
			sizeof(uint32_t),
			sizeof(int)
		];
		
		AssignHostPacket assignHostPacket = (*static_cast<AssignHostPacket*>(packet.get()));
		size_t offset = 0;

		Type type = assignHostPacket.GetType();
		memcpy(buffer, &type, sizeof(Type));
		offset = offset + sizeof(Type);

		uint8_t channel = assignHostPacket.GetChannel();
		memcpy(buffer + offset, &channel, sizeof(uint8_t));
		offset = offset + sizeof(uint8_t);

		uint32_t sequenceNumber = assignHostPacket.GetSequenceNumber();
		memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
		offset = offset + sizeof(uint32_t);

		int hostID = assignHostPacket.GetHostID();
		memcpy(buffer + offset, &hostID, sizeof(int));
		offset = offset + sizeof(int);

		int packetFlags = 0;
		if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
		else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

		ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
		return enetPacket;
	}
#pragma endregion AssignHostPacketHandler


#pragma region UserInfoPacketHandler
	void UserInfoPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
		const UserInfoPacket* userInfo = std::static_pointer_cast<UserInfoPacket>(packet).get();
		std::optional<Lobby>& lobby = TutorialGame::GetLobby();

		if (!lobby.has_value()) return;

		switch (userInfo->GetAction()) {
		case LobbyAction::CREATE:
			TutorialGame::SetUser(userInfo->GetUser());
			break;
		case LobbyAction::JOIN:
			lobby.value().AddUser(userInfo->GetUser());
			break;
		case LobbyAction::LEAVE:
			lobby.value().RemoveUser(userInfo->GetUser());
			break;
		}
	}

	std::shared_ptr<Packet> UserInfoPacketHandler::Translate(const ENetEvent* event) const {
		return std::make_shared<Packet>();

	}

	ENetPacket* UserInfoPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		return nullptr;
	}
#pragma endregion UserInfoPacketHandler


#pragma region RequestUserIDPacketHandler
	void RequestUserIDPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
	
	}

	std::shared_ptr<Packet> RequestUserIDPacketHandler::Translate(const ENetEvent* event) const {
		return std::make_shared<Packet>();

	}

	ENetPacket* RequestUserIDPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		return nullptr;
	}
#pragma endregion RequestUserIDPacketHandler
}
