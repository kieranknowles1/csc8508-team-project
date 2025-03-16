#include <chrono>
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

        if (isHost) {
            m_user = new Lobbies::User(m_uniqueUserID++);
            m_lobby->AddUser(*m_user);
            m_lobby->SetHost(*m_user);
        }

        ENetAddress networkAddr;
        networkAddr.host = ENET_HOST_ANY;
        networkAddr.port = isHost ? DEFAULT_PORT : 0;

        // Max number of incoming connections is 1 less (because of host).
        m_network = new Network(&networkAddr, isHost ? MAX_PLAYERS - 1 : 1);
        m_network->AddTickListener([&](bool endOfTick) { SendState(endOfTick); });
        m_network->AddTickListener([&](bool endOfTick) { ProcessPackets(endOfTick); });

        if (isHost) {
            m_network->SetConnectCallback([&](ENetPeer* client) { OnClientJoin(client); });
        }

    }

    Server::~Server() {
        m_network->Close();

        delete m_network;
        delete m_lobby;
        delete m_user;
    }

    void Server::InitPacketHandlers() {
        m_handlers.push_back(std::make_unique<Packet::UserInfoPacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());

        m_handlers.push_back(std::make_unique<Packet::PositionPacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());

        m_handlers.push_back(std::make_unique<Packet::DeltaPacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());

        m_handlers.push_back(std::make_unique<Packet::StartGamePacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());

        m_handlers.push_back(std::make_unique<Packet::ObjectChangeGravityPacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());

        m_handlers.push_back(std::make_unique<Packet::DamagePacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());
    }

    void Server::JoinGame(const std::string& ip, float waitSeconds) {
        ENetAddress destination = { 0, DEFAULT_PORT };
        enet_address_set_host(&destination, ip.c_str());

        m_network->ConnectTo(&destination);
        auto start = std::chrono::high_resolution_clock::now();
        std::chrono::steady_clock::time_point end;
        std::chrono::duration<float> elapsed;

        do {
            end = std::chrono::high_resolution_clock::now();
            elapsed = end - start;
        } while (m_network->GetConnectionCount() < 1 && elapsed.count() < waitSeconds);

        m_connected = m_network->GetConnectionCount() > 0;
    }

    void Server::SetUser(const Lobbies::User& user) {
        if (m_user != nullptr) {
            delete m_user;
        }
        m_user = new Lobbies::User(user);
    }

    void Server::SendState(bool endOfTick) {
        if (endOfTick) return;
        if (!m_isHost) return;
        
        if (m_game->GetState() == GameState::STARTING) {
            std::shared_ptr<Packet::StartGamePacket> startGame = std::make_shared<Packet::StartGamePacket>();
            m_network->Broadcast(startGame);
            m_game->SetState(GameState::ACTIVE);
        }

        if (m_game->GetState() != GameState::ACTIVE) return;

        m_game->GetWorld()->OperateOnContents([&](GameObject* object) {
            if (object->isStatic()) return;
            if (!object->GetWorldState().HasState()) return;

            WorldState::ObjectState& worldState = object->GetWorldState();
            worldState.AcquireReadLock(); // ACQUIRE LOCK.

            // TODO: Move packet creation to virtual function inside GameObject.
            // Create packets.
            std::shared_ptr<Packet::DeltaPacket> delta = std::make_shared<Packet::DeltaPacket>(
                object->GetWorldID(),
                std::get<btVector3>(worldState.UnsafeReadState(WorldState::StateType::LinearVelocity)),
                std::get<btVector3>(worldState.UnsafeReadState(WorldState::StateType::AngularVelocity)),
                m_tickCount
            );
            m_network->Broadcast(delta);

            std::shared_ptr<Packet::PositionPacket> position = std::make_shared<Packet::PositionPacket>(
                object->GetWorldID(),
                std::get<btVector3>(worldState.UnsafeReadState(WorldState::StateType::Position)),
                std::get<btQuaternion>(worldState.UnsafeReadState(WorldState::StateType::Rotation)),
                m_tickCount
            );
            m_network->Broadcast(position);

            // Create change gravity packet for players.
            if (object->getType() == GameObject::Type::Player) {
                PlayerObject* playerObj = static_cast<PlayerObject*>(object);

                std::shared_ptr<Packet::ObjectChangeGravityPacket> gravity = std::make_shared<Packet::ObjectChangeGravityPacket>(
                    playerObj->GetWorldID(),
                    std::get<btVector3>(worldState.UnsafeReadState(WorldState::StateType::UpVector)),
                    m_tickCount
                );
                m_network->Broadcast(gravity);
            }
            worldState.ReleaseReadLock(); // RELEASE LOCK.
        });
    }

    void Server::ProcessPackets(bool endOfTick) {
        if (!endOfTick) return;

        // TODO: place packets into a buffer to add a little delay before processing so that
        // enough time has passed for all the packets to arrive.
        std::shared_ptr<Packet::Packet> currentPacket = m_network->Fetch();
        while (currentPacket.get() != nullptr) {
            Packet::PacketRegister::GetHandler(currentPacket->GetType())->Handle(currentPacket);
            currentPacket = m_network->Fetch();
        }
    }

    void Server::OnClientJoin(ENetPeer* client) {
        // Make new user.
        Lobbies::User clientUser(m_uniqueUserID++);
        std::shared_ptr<Packet::UserInfoPacket> newUser = std::make_shared<Packet::UserInfoPacket>(
            clientUser,
            LobbyAction::CREATE
        );
        m_network->Send(newUser, client);
        m_lobby->AddUser(clientUser);

        // Send current lobby information.
        for (const User& player : m_lobby->GetConnectedUsers()) {
            std::shared_ptr<Packet::UserInfoPacket> playerInfo = std::make_shared<Packet::UserInfoPacket>(
                player,
                LobbyAction::JOIN
            );
            m_network->Send(playerInfo, client);
        }
        // NOTE: There is no need to send world info here because currently the
        // Lobby is just a menu screen.
    }
}


