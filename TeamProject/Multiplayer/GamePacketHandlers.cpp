#include "TutorialGame.h"
#include "GameObject.h"

#include "Multiplayer/GamePacketHandlers.hpp"

namespace Packet {
	void DeltaPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
		const DeltaPacket* deltaPacket = static_cast<const DeltaPacket*>(packet.get());
		GameObject* object = GameObject::GetGameObjectByID(deltaPacket->GetTargetID());

		object->GetPhysicsObject()->GetRigidBody()->setLinearVelocity(deltaPacket->GetLinearVelocity());
		object->GetPhysicsObject()->GetRigidBody()->setAngularVelocity(deltaPacket->GetAngularVelocity());
	}
	
	std::shared_ptr<Packet> DeltaPacketHandler::Translate(const ENetPacket* packet) const {
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
		//char* buffer = new char[sizeof(DeltaPacket)];
		//DeltaPacket deltaPacket = (*static_cast<DeltaPacket*>(packet.get()));

		//memcpy(buffer, &deltaPacket, sizeof(deltaPacket));

		return nullptr;
	}


	void PositionPacketHandler::Handle(const std::shared_ptr<Packet> packet) {

	}

	std::shared_ptr<Packet> PositionPacketHandler::Translate(const ENetPacket* packet) const {
		return std::make_shared<Packet>();
	}

	ENetPacket* PositionPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		return nullptr;
	}


	void PlayerStateChangePacketHandler::Handle(const std::shared_ptr<Packet> packet) {

	}

	std::shared_ptr<Packet> PlayerStateChangePacketHandler::Translate(const ENetPacket* packet) const {
		return std::make_shared<Packet>();
	}

	ENetPacket* PlayerStateChangePacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		return nullptr;
	}


	void ObjectChangeGravityPacketHandler::Handle(const std::shared_ptr<Packet> packet) {

	}

	std::shared_ptr<Packet> ObjectChangeGravityPacketHandler::Translate(const ENetPacket* packet) const {
		return std::make_shared<Packet>();
	}

	ENetPacket* ObjectChangeGravityPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
		return nullptr;
	}
}
