#include "TutorialGame.h"
#include "GameObject.h"
#include "Multiplayer/GamePacketHandlers.hpp"
#include "Multiplayer/Server.hpp"
#include "WorldState.h"
#include "Shoot.h"
#include "Network/Network.hpp"
#include "Health.h"
#include "Score.h"
#include <AudioEngine.h>

std::unordered_map <int, int> Packet::PacketHandler::activeChannelPerObject;
std::unordered_map <int, std::string> Packet::PacketHandler::activeSoundPerObject;

using namespace Lobbies;

namespace Packet {
    using namespace WorldState;

#pragma region DeltaPacketHandler
    void DeltaPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const DeltaPacket* deltaPacket = std::static_pointer_cast<DeltaPacket>(packet).get();
        ServerObject* object = (ServerObject*) ServerObject::GetGameObjectByID(deltaPacket->GetTargetID());

        if (object == nullptr) return;
        if (object->GetOwner() == nullptr) return;
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
        LaserObject* object = (LaserObject*) ServerObject::GetGameObjectByID(laserPacket->GetTargetID());

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
        ServerObject* object = (ServerObject*) ServerObject::GetGameObjectByID(positionPacket->GetTargetID());

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
        GameObject* hitBy = GameObject::GetGameObjectByID(damagePacket->GetDamageDealer());

        if (targetObject == nullptr) return;

        // NOTE: this is not threadsafe.
        HealthAttrib* health = targetObject->GetHealthAttrib();
        health->SetLastHitBy(hitBy);
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


#pragma region DeathPacketHandler
    void DeathPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const DeathPacket* scorePacket = std::static_pointer_cast<DeathPacket>(packet).get();
        GameObject* object = GameObject::GetGameObjectByID(scorePacket->GetObjectID());

        if (object == nullptr) return;

        if (object->getType() == GameObject::Type::Player) {
            // NOTE: this is not threadsafe.
            ScoreAttrib* scoreAttrib = ((PlayerObject*)object)->GetScoreAttrib();
            scoreAttrib->AddToScore(scorePacket->GetScoreIncrease());
        }
    }
    
    std::shared_ptr<Packet> DeathPacketHandler::Translate(const ENetEvent* event) const {
        ENetPacket* packet = event->packet;
        Type type;
        uint8_t channel;
        uint32_t sequenceNumber;
        
        int objectID;
        float score;
        btVector3 linearVelocity;
        btVector3 angularVelocity;
        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

        GetBaseData(packet, &type, &channel, &sequenceNumber);

        memcpy(&objectID, packet->data + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&score, packet->data + offset, sizeof(float));
        offset += sizeof(float);

        return std::make_shared<DeathPacket>(objectID, score, sequenceNumber);
    }
    
    ENetPacket* DeathPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        char* buffer = new char[
            sizeof(Type)
            + sizeof(uint8_t)
            + sizeof(uint32_t)
            + sizeof(int)
            + sizeof(float)
        ];

        DeathPacket scorePacket = (*static_cast<DeathPacket*>(packet.get()));
        size_t offset = 0;

        Type type = scorePacket.GetType();
        memcpy(buffer, &type, sizeof(Type));
        offset = offset + sizeof(Type);

        uint8_t channel = scorePacket.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint32_t sequenceNumber = scorePacket.GetSequenceNumber();
        memcpy(buffer + offset, &sequenceNumber, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        int objectID = scorePacket.GetObjectID();
        memcpy(buffer + offset, &objectID, sizeof(int));
        offset = offset + sizeof(int);

        float score = scorePacket.GetScoreIncrease();
        memcpy(buffer + offset, &score, sizeof(int));
        offset = offset + sizeof(int);

        int packetFlags = 0;
        if (channel == static_cast<int>(Channel::RELIABLE)) packetFlags = ENET_PACKET_FLAG_RELIABLE;
        else if (channel == static_cast<int>(Channel::UNSEQUENCED)) packetFlags = ENET_PACKET_FLAG_UNSEQUENCED;

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, packetFlags);
        delete[] buffer;
        return enetPacket;
    }


#pragma endregion DeathPacketHandler


#pragma region SoundPacketHandler

    void SoundPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        auto sound = std::static_pointer_cast<SoundPacket>(packet);
        int objectID = sound->GetObjectID();

        /*if (sound->GetSoundName() == "Beam.mp3") {
            auto it = activeSoundPerObject.find(objectID);

            if (it != activeSoundPerObject.end() && it->second == "Beam.mp3") {
                return;
            }
        }*/
        GameObject* obj = GameObject::GetGameObjectByID(objectID);
        int localID = TutorialGame::getInstance()->GetLocalPlayerID();

        if (objectID == localID) return;
        
        if (!obj) return;

        Vector3 pos(sound->GetPosition().x(), sound->GetPosition().y(), sound->GetPosition().z());

        auto it = PacketHandler::activeChannelPerObject.find(objectID);
        if (it != PacketHandler::activeChannelPerObject.end()) {
            int existingChannel = it->second;
            if (audioEngine.IsPlaying(existingChannel)) {
                // Already playing, don’t play again
                return;
            }
            else {
                // Cleanup if the sound ended naturally
                PacketHandler::activeChannelPerObject.erase(objectID);
            }
        }
        float volume = sound->GetVolume();
        float playbackTime = sound->GetPlaybackTime();

        int channel = audioEngine.PlaySounds(sound->GetSoundName(), pos, volume);
        if (channel != -1) {
            audioEngine.SetChannel3dPosition(channel, pos);
            audioEngine.SetChannelPlaybackPosition(channel, static_cast<unsigned int>(playbackTime * 1000));
            obj->SetSoundChannelID(channel);
            //audioEngine.Set3dListenerAndOrientation(TutorialGame::getInstance()->getMainCam()->GetPosition(), TutorialGame::getInstance()->getPlayerObject()->getForwardDirection(), TutorialGame::getInstance()->getPlayerObject()->getUpDirection());
            audioEngine.SetChannel3dMinMaxDistance(channel, 100.0f, 700.0f);
            PacketHandler::activeChannelPerObject[objectID] = channel;
            PacketHandler::activeSoundPerObject[objectID] = sound->GetSoundName();
        }

        
        /*audioEngine.PlaySounds(
            sound->GetSoundName(),
            Vector3(sound->GetPosition().x(), sound->GetPosition().y(), sound->GetPosition().z()),
            sound->GetVolume()
        );*/
    }

    std::shared_ptr<Packet> SoundPacketHandler::Translate(const ENetEvent* event) const {
        ENetPacket* pkt = event->packet;
        Type type;
        uint8_t channel;
        uint32_t sequence;

        GetBaseData(pkt, &type, &channel, &sequence);

        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

        btVector3 position;
        memcpy(&position, pkt->data + offset, sizeof(btVector3));
        offset += sizeof(btVector3);

        float volume, pitch, playbackTime;
        memcpy(&volume, pkt->data + offset, sizeof(float));
        offset += sizeof(float);
        memcpy(&pitch, pkt->data + offset, sizeof(float));
        offset += sizeof(float);

        int objectID;
        memcpy(&objectID, pkt->data + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&playbackTime, pkt->data + offset, sizeof(float));
        offset += sizeof(float);

        int nameLen;
        memcpy(&nameLen, pkt->data + offset, sizeof(int));
        offset += sizeof(int);

        std::string soundName(reinterpret_cast<char*>(pkt->data + offset), nameLen);

        return std::make_shared<SoundPacket>(soundName, position, volume, pitch, playbackTime, objectID);
    }

    ENetPacket* SoundPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        auto sound = *static_cast<SoundPacket*>(packet.get());

        //int nameLen = (int)sound.GetSoundName().size();
        int nameLen = static_cast<int>(sound.GetSoundName().size());
        size_t totalSize =
            sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t) +
            sizeof(btVector3) + sizeof(float) + sizeof(float) +
            sizeof(int) + sizeof(float) + sizeof(int) + nameLen;

        char* buffer = new char[totalSize];
        size_t offset = 0;

        Type type = sound.GetType();
        memcpy(buffer + offset, &type, sizeof(Type)); offset += sizeof(Type);
        uint8_t channel = sound.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t)); offset += sizeof(uint8_t);
        uint32_t seq = sound.GetSequenceNumber();
        memcpy(buffer + offset, &seq, sizeof(uint32_t)); offset += sizeof(uint32_t);

        btVector3 pos = sound.GetPosition();
        memcpy(buffer + offset, &pos, sizeof(btVector3)); offset += sizeof(btVector3);
        float vol = sound.GetVolume();
        memcpy(buffer + offset, &vol, sizeof(float)); offset += sizeof(float);
        float pitch = sound.GetPitch();
        memcpy(buffer + offset, &pitch, sizeof(float)); offset += sizeof(float);

        int objID = sound.GetObjectID();
        memcpy(buffer + offset, &objID, sizeof(int)); offset += sizeof(int);

        float playback = sound.GetPlaybackTime();
        memcpy(buffer + offset, &playback, sizeof(float)); offset += sizeof(float);

        //int len = (int)sound.GetSoundName().size();
        int len = static_cast<int>(sound.GetSoundName().size());
        memcpy(buffer + offset, &len, sizeof(int)); offset += sizeof(int);
        memcpy(buffer + offset, sound.GetSoundName().c_str(), len); offset += len;

        ENetPacket* pkt = enet_packet_create(buffer, offset, ENET_PACKET_FLAG_RELIABLE);
        delete[] buffer;
        return pkt;
    }

#pragma endregion SoundPacketHandler

#pragma region SoundUpdatePacketHandler

    void SoundUpdatePacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const auto* soundUpdate = static_cast<SoundUpdatePacket*>(packet.get());
        int objectID = soundUpdate->GetSourceID();
        const btVector3& pos = soundUpdate->GetPosition();

        GameObject* obj = GameObject::GetGameObjectByID(objectID);
        if (!obj) return;

        
        int channel = obj->GetChannelID();

        if (channel != -1) {
            audioEngine.SetChannel3dPosition(channel, Vector3(pos.x(), pos.y(), pos.z()));
        }
    }

    std::shared_ptr<Packet> SoundUpdatePacketHandler::Translate(const ENetEvent* event) const {
        Type type;
        uint8_t channel;
        uint32_t sequence;
        int objectID;
        btVector3 position;

        GetBaseData(event->packet, &type, &channel, &sequence);
        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

        memcpy(&objectID, event->packet->data + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&position, event->packet->data + offset, sizeof(btVector3));

        return std::make_shared<SoundUpdatePacket>(objectID, position);
    }

    ENetPacket* SoundUpdatePacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        const SoundUpdatePacket& sound = *static_cast<SoundUpdatePacket*>(packet.get());

        char* buffer = new char[
            sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t) +
                sizeof(int) + sizeof(btVector3)
        ];

        size_t offset = 0;

        Type type = sound.GetType();
        memcpy(buffer + offset, &type, sizeof(Type)); offset += sizeof(Type);

        uint8_t channel = sound.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t)); offset += sizeof(uint8_t);

        uint32_t seq = sound.GetSequenceNumber();
        memcpy(buffer + offset, &seq, sizeof(uint32_t)); offset += sizeof(uint32_t);

        int objID = sound.GetSourceID();
        memcpy(buffer + offset, &objID, sizeof(int)); offset += sizeof(int);

        btVector3 pos = sound.GetPosition();
        memcpy(buffer + offset, &pos, sizeof(btVector3)); offset += sizeof(btVector3);

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, ENET_PACKET_FLAG_UNSEQUENCED);
        delete[] buffer;
        return enetPacket;
    }

#pragma endregion SoundUpdatePacketHandler

#pragma region StopSoundPacketHandler

    void StopSoundPacketHandler::Handle(const std::shared_ptr<Packet> packet) {
        const auto* stopSound = static_cast<StopSoundPacket*>(packet.get());
        int objectID = stopSound->GetObjectID();

        GameObject* obj = GameObject::GetGameObjectByID(objectID);
        if (!obj) return;

        if (obj->HasAudioChannel()) {
            int channel = obj->GetChannelID();
            //audioEngine.SetChannelVolume(channel, -100.0f);
            //obj->SetSoundChannelID(-1);
            audioEngine.StopChannel(channel);
            obj->ClearSoundChannel();

            
        }
        PacketHandler::activeChannelPerObject.erase(objectID);
        PacketHandler::activeSoundPerObject.erase(objectID);
    }

    std::shared_ptr<Packet> StopSoundPacketHandler::Translate(const ENetEvent* event) const {
        Type type;
        uint8_t channel;
        uint32_t sequence;
        int objectID;

        GetBaseData(event->packet, &type, &channel, &sequence);
        size_t offset = sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t);

        memcpy(&objectID, event->packet->data + offset, sizeof(int));

        return std::make_shared<StopSoundPacket>(objectID);
    }

    ENetPacket* StopSoundPacketHandler::ToENetPacket(const std::shared_ptr<Packet> packet) const {
        const StopSoundPacket& sound = *static_cast<StopSoundPacket*>(packet.get());

        char* buffer = new char[
            sizeof(Type) + sizeof(uint8_t) + sizeof(uint32_t) +
                sizeof(int)
        ];

        size_t offset = 0;

        Type type = sound.GetType();
        memcpy(buffer + offset, &type, sizeof(Type)); offset += sizeof(Type);

        uint8_t channel = sound.GetChannel();
        memcpy(buffer + offset, &channel, sizeof(uint8_t)); offset += sizeof(uint8_t);

        uint32_t seq = sound.GetSequenceNumber();
        memcpy(buffer + offset, &seq, sizeof(uint32_t)); offset += sizeof(uint32_t);

        int objID = sound.GetObjectID();
        memcpy(buffer + offset, &objID, sizeof(int)); offset += sizeof(int);

        ENetPacket* enetPacket = enet_packet_create(buffer, offset, ENET_PACKET_FLAG_UNSEQUENCED);
        delete[] buffer;
        return enetPacket;
    }

#pragma endregion StopSoundPacketHandler
}
