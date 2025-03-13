#include "TutorialGame.h"
#include "GameObject.h"

#include "Multiplayer/GamePacketHandlers.hpp"

namespace Packet {

#pragma region DeltaPacketHandler
    void DeltaPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const DeltaPacket* deltaPacket = std::static_pointer_cast<DeltaPacket>(packet).get();
        GameObject* object = GameObject::GetGameObjectByID(deltaPacket->GetTargetID());

        // Skip updates for objects the user owns.
        if (object->GetOwner().value() == TutorialGame::GetUser().value()) { return; }

        // Check if last update was newer.
        if (deltaPacket->GetSequenceNumber() > object->GetLastPacketSequence(deltaPacket->GetType())) {
            object->GetPhysicsObject()->GetRigidBody()->setLinearVelocity(deltaPacket->GetLinearVelocity());
            object->GetPhysicsObject()->GetRigidBody()->setAngularVelocity(deltaPacket->GetAngularVelocity());
            object->UpdatePacketSequence(deltaPacket->GetType(), deltaPacket->GetSequenceNumber());

            // Passing on packet to other users if user is host.
            if (TutorialGame::IsHost()) TutorialGame::GetServerInstance()->Broadcast(packet);
        }
        // Dropping old packets.
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


        // Skip updates for objects the user owns.
        if (targetObject->GetOwner().value() == TutorialGame::GetUser().value()) return; 

        // Check if last update was newer.
        if (positionPacket->GetSequenceNumber() > targetObject->GetLastPacketSequence(positionPacket->GetType())) {
            body->getWorldTransform().setOrigin(positionPacket->GetPosition());
            body->getWorldTransform().setRotation(positionPacket->GetOrientation());
            targetObject->UpdatePacketSequence(positionPacket->GetType(), positionPacket->GetSequenceNumber());

            // Passing on packet to other users if user is host.
            if (TutorialGame::IsHost()) TutorialGame::GetServerInstance()->Broadcast(packet);
        }
        // Dropping old packets.
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
        const ObjectChangeGravityPacket* gravityPacket = std::static_pointer_cast<ObjectChangeGravityPacket>(packet).get();
        PlayerObject* targetObject = (PlayerObject*) GameObject::GetGameObjectByID(gravityPacket->GetTargetID());

        // Skip updates for objects the user owns.
        if (targetObject->GetOwner().value() == TutorialGame::GetUser().value()) return;

        // Check if last update was newer.
        if (gravityPacket->GetSequenceNumber() > targetObject->GetLastPacketSequence(gravityPacket->GetType())) {
            targetObject->setUpDirection(gravityPacket->GetUpDirection());
            targetObject->UpdatePacketSequence(gravityPacket->GetType(), gravityPacket->GetSequenceNumber());

            // Passing on packet to other users if user is host.
            if (TutorialGame::IsHost()) TutorialGame::GetServerInstance()->Broadcast(packet);
        }
    }

    std::shared_ptr<Packet> ObjectChangeGravityPacketHandler::Translate(const ENetEvent* event) const {
        ENetPacket* packet = event->packet;
        Type type;
        uint8_t channel;
        uint32_t sequenceNumber;
        
        int objectID;
        btVector3 upVector;
        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

        GetBaseData(packet, &type, &channel, &sequenceNumber);

        memcpy(&objectID, packet->data + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&upVector, packet->data + offset, sizeof(btVector3));
        offset += sizeof(btVector3);

        return std::make_shared<ObjectChangeGravityPacket>(objectID, upVector, sequenceNumber);
    }

    ENetPacket* ObjectChangeGravityPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        char* buffer = new char[
            sizeof(Type)
            + sizeof(uint8_t)
            + sizeof(uint32_t)
            + sizeof(int)
            + sizeof(btVector3)
            + sizeof(btQuaternion)
        ];

        ObjectChangeGravityPacket gravityPacket = (*static_cast<ObjectChangeGravityPacket*>(packet.get()));
        size_t offset = 0;

        Type type = gravityPacket.GetType();
        memcpy(buffer, &type, sizeof(Type));
        offset = offset + sizeof(Type);

        uint8_t channel = gravityPacket.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint32_t sequenceNumber = gravityPacket.GetSequenceNumber();
        memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        int objectID = gravityPacket.GetTargetID();
        memcpy(buffer + offset, &objectID, sizeof(int));
        offset = offset + sizeof(int);

        btVector3 upVector = gravityPacket.GetUpDirection();
        memcpy(buffer + offset, &upVector, sizeof(btVector3));
        offset = offset + sizeof(btVector3);

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        return enetPacket;
    }
#pragma endregion ObjectChangeGravityPacketHandler


#pragma region StartGamePacketHandler
    void StartGamePacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        TutorialGame::Start();
    }

    std::shared_ptr<Packet> StartGamePacketHandler::Translate(const ENetEvent* event) const {
        return std::make_shared<StartGamePacket>();
    }

    ENetPacket* StartGamePacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        char* buffer = new char[
            sizeof(Type) +
            sizeof(uint8_t) +
            sizeof(uint32_t) +
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

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        return enetPacket;
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
            sizeof(Type) +
            sizeof(uint8_t) +
            sizeof(uint32_t) +
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
        TutorialGame::UpdateUserID(userInfo->GetUser().GetUserID());

        if (!lobby.has_value()) return;

        switch (userInfo->GetAction()) {
        case LobbyAction::CREATE:
            TutorialGame::SetUser(userInfo->GetUser());
            break;
        case LobbyAction::JOIN:
            lobby->AddUser(userInfo->GetUser());
            break;
        case LobbyAction::LEAVE:
            lobby->RemoveUser(userInfo->GetUser());
            break;
        case LobbyAction::SET_HOST:
            lobby->SetHost(userInfo->GetUser());
        }
    }

    std::shared_ptr<Packet> UserInfoPacketHandler::Translate(const ENetEvent* event) const {
        ENetPacket* packet = event->packet;
        Type type;
        uint8_t channel;
        uint32_t sequenceNumber;
        
        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

        GetBaseData(packet, &type, &channel, &sequenceNumber);

        LobbyAction action;
        memcpy(&action, packet->data + packet->dataLength - 1, sizeof(LobbyAction));

        char* data = (char*)(packet->data + sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t));
        User user = User::Deserialize(data);

        return std::make_shared<UserInfoPacket>(user, action);
    }

    ENetPacket* UserInfoPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        UserInfoPacket userInfo = (*static_cast<UserInfoPacket*>(packet.get()));

        char* buffer = new char[
            sizeof(Type) +
            sizeof(uint8_t) +
            sizeof(uint32_t) +
            sizeof(userInfo.GetUser().Size()) +
            sizeof(uint8_t)
        ];
        size_t offset = 0;

        Type type = userInfo.GetType();
        memcpy(buffer, &type, sizeof(Type));
        offset = offset + sizeof(Type);

        uint8_t channel = userInfo.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint32_t sequenceNumber = userInfo.GetSequenceNumber();
        memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        char* userData = userInfo.GetUser().Serialize();
        memcpy(buffer + offset, userData, userInfo.GetUser().Size());
        offset = offset + userInfo.GetUser().Size();

        uint8_t action = static_cast<uint8_t>(userInfo.GetAction());
        memcpy(buffer + offset, &action, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        enetPacket->dataLength = offset;
        return enetPacket;
    }
#pragma endregion UserInfoPacketHandler


#pragma region RequestUserIDPacketHandler
    void RequestUserIDPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const RequestUserIDPacket* request = std::static_pointer_cast<RequestUserIDPacket>(packet).get();

        int newUserID = TutorialGame::GenerateUserID();
        User newUser(newUserID);
        
        std::shared_ptr<UserInfoPacket> infoPacket = std::make_shared<UserInfoPacket>(newUser, LobbyAction::CREATE);
        TutorialGame::GetServerInstance()->Send(infoPacket, request->GetPeer());

        std::shared_ptr<UserInfoPacket> hostPacket = std::make_shared<UserInfoPacket>(TutorialGame::GetUser().value(), LobbyAction::SET_HOST);
        TutorialGame::GetServerInstance()->Send(hostPacket, request->GetPeer());

        // Insert into host lobby. Only server creator (host) receives Request packets.
        TutorialGame::GetLobby()->AddUser(newUser);

        // Send information about every user in the lobby.
        for (User user : TutorialGame::GetLobby()->GetConnectedUsers()) {
            infoPacket = std::make_shared<UserInfoPacket>(user, LobbyAction::JOIN);
            TutorialGame::GetServerInstance()->Broadcast(infoPacket);
        }
    }

    std::shared_ptr<Packet> RequestUserIDPacketHandler::Translate(const ENetEvent* event) const {
        return std::make_shared<RequestUserIDPacket>(event->peer);
    }

    ENetPacket* RequestUserIDPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        RequestUserIDPacket request = (*static_cast<RequestUserIDPacket*>(packet.get()));

        char* buffer = new char[
            sizeof(Type) +
            sizeof(uint8_t) +
            sizeof(uint32_t)
        ];
        size_t offset = 0;

        Type type = request.GetType();
        memcpy(buffer, &type, sizeof(Type));
        offset = offset + sizeof(Type);

        uint8_t channel = request.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint32_t sequenceNumber = request.GetSequenceNumber();
        memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        return enetPacket;
    }
#pragma endregion RequestUserIDPacketHandler
}
