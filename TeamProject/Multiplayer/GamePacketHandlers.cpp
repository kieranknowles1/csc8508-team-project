#include "TutorialGame.h"
#include "GameObject.h"

#include "Multiplayer/GamePacketHandlers.hpp"

namespace Packet {
	void DeltaPacketHandler::Handle(const Packet& packet) {
		const DeltaPacket& deltaPacket = static_cast<const DeltaPacket&>(packet);
		GameObject* object = GameObject::GetGameObjectByID(deltaPacket.GetTargetID());

		object->GetPhysicsObject()->GetRigidBody()->setLinearVelocity(deltaPacket.GetLinearVelocity());
		object->GetPhysicsObject()->GetRigidBody()->setAngularVelocity(deltaPacket.GetAngularVelocity());
	}
	
	Packet DeltaPacketHandler::Translate(ENetPacket* packet) const {
		return Packet();
	}
	
	ENetPacket* DeltaPacketHandler::ToENetPacket(const Packet& packet) const {
		return nullptr;
	}


	void PositionPacketHandler::Handle(const Packet& packet) {

	}

	Packet PositionPacketHandler::Translate(ENetPacket* packet) const {
		return Packet();
	}

	ENetPacket* PositionPacketHandler::ToENetPacket(const Packet& packet) const {
		return nullptr;
	}


	void PlayerStateChangePacketHandler::Handle(const Packet& packet) {

	}

	Packet PlayerStateChangePacketHandler::Translate(ENetPacket* packet) const {
		return Packet();
	}

	ENetPacket* PlayerStateChangePacketHandler::ToENetPacket(const Packet& packet) const {
		return nullptr;
	}


	void ObjectChangeGravityPacketHandler::Handle(const Packet& packet) {

	}

	Packet ObjectChangeGravityPacketHandler::Translate(ENetPacket* packet) const {
		return Packet();
	}

	ENetPacket* ObjectChangeGravityPacketHandler::ToENetPacket(const Packet& packet) const {
		return nullptr;
	}
}
