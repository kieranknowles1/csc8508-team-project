#include "TutorialGame.h"
#include "GameObject.h"
#include "Multiplayer/GamePacketHandlers.hpp"
#include "Multiplayer/Server.hpp"
#include "WorldState.h"
#include "Shoot.h"
#include "Network/Network.hpp"
#include "Health.h"

using namespace Lobbies;

namespace Packet {
    using namespace WorldState;

#pragma region DeltaPacketHandler
    void DeltaPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const DeltaPacket* deltaPacket = std::static_pointer_cast<DeltaPacket>(packet).get();
        GameObject* object = GameObject::GetGameObjectByID(deltaPacket->GetTargetID());

        if (object == nullptr) return;

        if (TutorialGame::getInstance()->GetServerInstance()->IsOwnerOf(object)) return;
        auto [writeState, lock] = object->GetWorldStates()->GetWriteState();

        std::unique_lock stateLock = writeState->Lock();
        writeState->UpdateState(StateType::LinearVelocity, deltaPacket->GetLinearVelocity());
        writeState->UpdateState(StateType::AngularVelocity, deltaPacket->GetAngularVelocity());
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
        delete[] buffer;
        return enetPacket;
    }
#pragma endregion DeltaPacketHandler


#pragma region LaserPacketHandler
    void LaserPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const LaserPacket* laserPacket = std::static_pointer_cast<LaserPacket>(packet).get();
        LaserObject* object = (LaserObject*)GameObject::GetGameObjectByID(laserPacket->GetTargetID());

        if (object == nullptr) return;

        if (TutorialGame::getInstance()->GetServerInstance()->IsOwnerOf(object)) return;

        auto [writeState, lock] = object->GetWorldStates()->GetWriteState();

        std::unique_lock stateLock = writeState->Lock();
        writeState->UpdateState(StateType::StartPos, laserPacket->GetStartPos());
        writeState->UpdateState(StateType::EndPos, laserPacket->GetEndPos());
        writeState->UpdateState(StateType::Normal, laserPacket->GetHitNormal());
    }

    std::shared_ptr<Packet> LaserPacketHandler::Translate(const ENetEvent* event) const {
        ENetPacket* packet = event->packet;
        Type type;
        uint8_t channel;
        uint32_t sequenceNumber;

        int objectID;
        btVector3 startPos;
        btVector3 endPos;
        btVector3 hitNormal;
        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

        GetBaseData(packet, &type, &channel, &sequenceNumber);

        memcpy(&objectID, packet->data + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&startPos, packet->data + offset, sizeof(btVector3));
        offset += sizeof(btVector3);

        memcpy(&endPos, packet->data + offset, sizeof(btVector3));
        offset += sizeof(btVector3);

        memcpy(&hitNormal, packet->data + offset, sizeof(btVector3));
        offset += sizeof(btVector3);

        return std::make_shared<LaserPacket>(objectID, startPos, endPos, hitNormal, sequenceNumber);
    }

    ENetPacket* LaserPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        char* buffer = new char[
            sizeof(Type)
                + sizeof(uint8_t)
                + sizeof(uint32_t)
                + sizeof(int)
                + sizeof(btVector3)
                + sizeof(btVector3)
                + sizeof(btVector3)
        ];

        LaserPacket laserPacket = (*static_cast<LaserPacket*>(packet.get()));
        size_t offset = 0;

        Type type = laserPacket.GetType();
        memcpy(buffer, &type, sizeof(Type));
        offset = offset + sizeof(Type);

        uint8_t channel = laserPacket.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint32_t sequenceNumber = laserPacket.GetSequenceNumber();
        memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        int objectID = laserPacket.GetTargetID();
        memcpy(buffer + offset, &objectID, sizeof(int));
        offset = offset + sizeof(int);

        btVector3 startPos = laserPacket.GetStartPos();
        memcpy(buffer + offset, &startPos, sizeof(btVector3));
        offset = offset + sizeof(btVector3);

        btVector3 endPos = laserPacket.GetEndPos();
        memcpy(buffer + offset, &endPos, sizeof(btVector3));
        offset = offset + sizeof(btVector3);

        btVector3 hitNormal = laserPacket.GetHitNormal();
        memcpy(buffer + offset, &hitNormal, sizeof(btVector3));
        offset = offset + sizeof(btVector3);

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        delete[] buffer;
        return enetPacket;
    }
#pragma endregion LaserPacketHandler


#pragma region PositonPacketHandler
    void PositionPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const PositionPacket* positionPacket = std::static_pointer_cast<PositionPacket>(packet).get();
        GameObject* object = GameObject::GetGameObjectByID(positionPacket->GetTargetID());

        if (object == nullptr) return;

        if (TutorialGame::getInstance()->GetServerInstance()->IsOwnerOf(object)) return;

        auto [writeState, lock] = object->GetWorldStates()->GetWriteState();
        
        std::unique_lock stateLock = writeState->Lock();
        writeState->UpdateState(StateType::Position, positionPacket->GetPosition());
        writeState->UpdateState(StateType::Rotation, positionPacket->GetOrientation());
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
        delete[] buffer;
        return enetPacket;
    }
#pragma endregion PositonPacketHandler


//#pragma region PlayerStateChangePacketHandler
//    void PlayerStateChangePacketHandler::Handle(const std::shared_ptr<Packet> packet) {
//
//    }
//
//    std::shared_ptr<Packet> PlayerStateChangePacketHandler::Translate(const ENetEvent* event) const {
//        return std::make_shared<Packet>();
//    }
//
//    ENetPacket* PlayerStateChangePacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
//        return nullptr;
//    }
//#pragma endregion PlayerStateChangePacketHandler


#pragma region ObjectChangeGravityPacketHandler
    void ObjectChangeGravityPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const ObjectChangeGravityPacket* gravityPacket = std::static_pointer_cast<ObjectChangeGravityPacket>(packet).get();
        PlayerObject* object = (PlayerObject*) GameObject::GetGameObjectByID(gravityPacket->GetTargetID());

        if (object == nullptr) return;
        if (TutorialGame::getInstance()->GetServerInstance()->IsOwnerOf(object)) return;

        auto [writeState, lock] = object->GetWorldStates()->GetWriteState();
        
        std::unique_lock stateLock = writeState->Lock();
        writeState->UpdateState(StateType::UpVector, gravityPacket->GetUpDirection());
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
        delete[] buffer;
        return enetPacket;
    }
#pragma endregion ObjectChangeGravityPacketHandler

#pragma region PlayerAnimationPacketHandler
    void PlayerAnimationPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const PlayerAnimationPacket* animationPacket = std::static_pointer_cast<PlayerAnimationPacket>(packet).get();
        PlayerObject* object = (PlayerObject*)GameObject::GetGameObjectByID(animationPacket->GetTargetID());

        if (object == nullptr) return;
        if (TutorialGame::getInstance()->GetServerInstance()->IsOwnerOf(object)) return;

        auto [writeState, lock] = object->GetWorldStates()->GetWriteState();

        std::unique_lock stateLock = writeState->Lock();
        writeState->UpdateState(StateType::Animation, animationPacket->GetAnimation());
    }

    std::shared_ptr<Packet> PlayerAnimationPacketHandler::Translate(const ENetEvent* event) const {
        ENetPacket* packet = event->packet;
        Type type;
        uint8_t channel;
        uint32_t sequenceNumber;

        int objectID;
        int animation;
        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

        GetBaseData(packet, &type, &channel, &sequenceNumber);

        memcpy(&objectID, packet->data + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&animation, packet->data + offset, sizeof(int));
        offset += sizeof(int);

        return std::make_shared<PlayerAnimationPacket>(objectID, animation, sequenceNumber);
    }

    ENetPacket* PlayerAnimationPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        char* buffer = new char[
            sizeof(Type)
                + sizeof(uint8_t)
                + sizeof(uint32_t)
                + sizeof(int)
                + sizeof(int)
        ];

        PlayerAnimationPacket animationPacket = (*static_cast<PlayerAnimationPacket*>(packet.get()));
        size_t offset = 0;

        Type type = animationPacket.GetType();
        memcpy(buffer, &type, sizeof(Type));
        offset = offset + sizeof(Type);

        uint8_t channel = animationPacket.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint32_t sequenceNumber = animationPacket.GetSequenceNumber();
        memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        int objectID = animationPacket.GetTargetID();
        memcpy(buffer + offset, &objectID, sizeof(int));
        offset = offset + sizeof(int);

        int animation = animationPacket.GetAnimation();
        memcpy(buffer + offset, &animation, sizeof(int));
        offset = offset + sizeof(int);

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        delete[] buffer;
        return enetPacket;
    }
#pragma endregion PlayerAnimationPacketHandler


#pragma region StartGamePacketHandler
    void StartGamePacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        TutorialGame* game = TutorialGame::getInstance();
        game->SetState(GameState::STARTING);
    }

    std::shared_ptr<Packet> StartGamePacketHandler::Translate(const ENetEvent* event) const {
        return std::make_shared<StartGamePacket>();
    }

    ENetPacket* StartGamePacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        char* buffer = new char[
            sizeof(Type) +
            sizeof(uint8_t) +
            sizeof(uint32_t)
        ];
        
        StartGamePacket startPacket = (*static_cast<StartGamePacket*>(packet.get()));
        size_t offset = 0;

        Type type = startPacket.GetType();
        memcpy(buffer, &type, sizeof(Type));
        offset = offset + sizeof(Type);

        uint8_t channel = startPacket.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint32_t sequenceNumber = startPacket.GetSequenceNumber();
        memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        delete[] buffer;
        return enetPacket;
    }
#pragma endregion StartGamePacketHandler


#pragma region UserInfoPacketHandler
    void UserInfoPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const UserInfoPacket* userInfo = std::static_pointer_cast<UserInfoPacket>(packet).get();
        Multiplayer::Server* server = TutorialGame::getInstance()->GetServerInstance();

        switch (userInfo->GetAction()) {
        case LobbyAction::CREATE:
            server->SetUser(userInfo->GetUser());
            break;
        case LobbyAction::JOIN:
            server->AddUserToLobby(userInfo->GetUser());
            break;
        case LobbyAction::LEAVE:
            server->RemoveUserFromLobby(userInfo->GetUser());
            break;
        case LobbyAction::SET_HOST:
            server->AssignLobbyHost(userInfo->GetUser());
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
        delete[] buffer;
        return enetPacket;
    }
#pragma endregion UserInfoPacketHandler


#pragma region DamageHandler
    void DamagePacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const DamagePacket* damagePacket = std::static_pointer_cast<DamagePacket>(packet).get();
        PlayerObject* targetObject = (PlayerObject*) GameObject::GetGameObjectByID(damagePacket->GetTargetID());

        if (targetObject == nullptr) return;

        HealthAttrib* health = targetObject->GetHealthAttrib();
        health->Damage(damagePacket->GetDamage());
    }

    std::shared_ptr<Packet> DamagePacketHandler::Translate(const ENetEvent* event) const {
        ENetPacket* packet = event->packet;
        Type type;
        uint8_t channel;
        uint32_t sequenceNumber;
        
        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);
        GetBaseData(packet, &type, &channel, &sequenceNumber);

        int targetID;
        memcpy(&targetID, packet->data + offset, sizeof(int));
        offset = offset + sizeof(int);

        float damage;
        memcpy(&damage, packet->data + offset, sizeof(float));
        offset = offset + sizeof(float);

        int dealer;
        memcpy(&dealer, packet->data + offset, sizeof(int));
        offset = offset + sizeof(int);

        return std::make_shared<DamagePacket>(targetID, damage, dealer);
    }

    ENetPacket* DamagePacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        char* buffer = new char[
            sizeof(Type)
            + sizeof(uint8_t)
            + sizeof(uint32_t)
            + sizeof(int)
            + sizeof(int)
            + sizeof(int)
        ];

        DamagePacket damagePacket = (*static_cast<DamagePacket*>(packet.get()));
        size_t offset = 0;

        Type type = damagePacket.GetType();
        memcpy(buffer, &type, sizeof(Type));
        offset = offset + sizeof(Type);

        uint8_t channel = damagePacket.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint32_t sequenceNumber = damagePacket.GetSequenceNumber();
        memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        int targetID = damagePacket.GetTargetID();
        memcpy(buffer + offset, &targetID, sizeof(int));
        offset = offset + sizeof(int);

        float damage = damagePacket.GetDamage();
        memcpy(buffer + offset, &damage, sizeof(float));
        offset = offset + sizeof(float);

        int dealer = damagePacket.GetDamageDealer();
        memcpy(buffer + offset, &dealer, sizeof(int));
        offset = offset + sizeof(int);

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        delete[] buffer;
        return enetPacket;
    }

#pragma endregion DamageHandler


#pragma region Ping
    void PingPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
    }

    std::shared_ptr<Packet> PingPacketHandler::Translate(const ENetEvent* event) const {
        ENetPacket* packet = event->packet;
        Type type;
        uint8_t channel;
        uint32_t sequenceNumber;

        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);
        GetBaseData(packet, &type, &channel, &sequenceNumber);

        return std::make_shared<PingPacket>();
    }

    ENetPacket* PingPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        char* buffer = new char[
            sizeof(Type)
            + sizeof(uint8_t)
            + sizeof(uint32_t)
        ];

        PingPacket pingPacket = (*static_cast<PingPacket*>(packet.get()));
        size_t offset = 0;

        Type type = pingPacket.GetType();
        memcpy(buffer, &type, sizeof(Type));
        offset = offset + sizeof(Type);

        uint8_t channel = pingPacket.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint32_t sequenceNumber = pingPacket.GetSequenceNumber();
        memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        delete[] buffer;
        return enetPacket;
    }


#pragma endregion Ping


#pragma region Pong
    void PongPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
    }

    std::shared_ptr<Packet> PongPacketHandler::Translate(const ENetEvent* event) const {
        ENetPacket* packet = event->packet;
        Type type;
        uint8_t channel;
        uint32_t sequenceNumber;

        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);
        GetBaseData(packet, &type, &channel, &sequenceNumber);

        return std::make_shared<PongPacket>();
    }

    ENetPacket* PongPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        char* buffer = new char[
            sizeof(Type)
            + sizeof(uint8_t)
            + sizeof(uint32_t)
        ];

        PongPacket pongPacket = (*static_cast<PongPacket*>(packet.get()));
        size_t offset = 0;

        Type type = pongPacket.GetType();
        memcpy(buffer, &type, sizeof(Type));
        offset = offset + sizeof(Type);

        uint8_t channel = pongPacket.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint32_t sequenceNumber = pongPacket.GetSequenceNumber();
        memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        delete[] buffer;
        return enetPacket;
    }
#pragma endregion Pong

}
