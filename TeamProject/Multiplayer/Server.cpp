#include <memory>

#include "Server.hpp"
#include "TutorialGame.h"

#include "Multiplayer/GamePackets.hpp"
#include "Multiplayer/GamePacketHandlers.hpp"
#include "Multiplayer/Lobby.hpp"
#include "Multiplayer/User.hpp"


namespace Multiplayer {
    Server::Server(TutorialGame* game, bool isHost) : m_game(game), m_isHost(isHost) {
        m_lobby = new Lobbies::Lobby(MAX_PLAYERS);

        int id = TutorialGame::GenerateUserID();
        m_user = new Lobbies::User(id);
        m_lobby->AddUser(*m_user);

        if (isHost) m_lobby->SetHost(*m_user);

        ENetAddress networkAddr;
        networkAddr.host = ENET_HOST_ANY;
        networkAddr.port = isHost ? DEFAULT_PORT : 0;

        m_network = new Network(&networkAddr, isHost ? MAX_PLAYERS : 1);
        m_network->AddTickListener([&](bool endOfTick) { SendState(endOfTick); });
        m_network->AddTickListener([&](bool endOfTick) { ProcessPackets(endOfTick); });

        m_network->Start();
    }

    Server::~Server() {
        m_network->Close();
        
        delete m_network;
        delete m_lobby;
        delete m_user;
    }

    void Server::SendState(bool endOfTick) {
        if (endOfTick) return;

        m_game->GetWorld()->OperateOnContents([&](GameObject* object) {
            if (object->isStatic()) return;

            btVector3 objLinearVelocity = object->getLinearVelocity();
            btVector3 objAngularVelocity = object->getAngularVelocity();
            btVector3 objPosition = object->getPosition();
            btQuaternion objOrientation = object->getRotation();

            // Create packets.
            std::shared_ptr<Packet::DeltaPacket> delta = std::make_shared<Packet::DeltaPacket>(
                object->GetWorldID(),
                objLinearVelocity,
                objAngularVelocity,
                m_tickCount
            );
            m_network->Broadcast(delta);

            std::shared_ptr<Packet::PositionPacket> position = std::make_shared<Packet::PositionPacket>(
                object->GetWorldID(),
                objPosition,
                objOrientation,
                m_tickCount
            );
            m_network->Broadcast(position);

            // Create change gravity packet for players.
            if (object->getType() == GameObject::Type::Player) {
                PlayerObject* playerObj = static_cast<PlayerObject*>(object);

                std::shared_ptr<Packet::ObjectChangeGravityPacket> gravity = std::make_shared<Packet::ObjectChangeGravityPacket>(
                    playerObj->GetWorldID(),
                    playerObj->getUpDirection(),
                    m_tickCount
                );
                m_network->Broadcast(gravity);
            }
        });
    }

    void Server::ProcessPackets(bool endOfTick) {
        if (!endOfTick) return;
    }





}


