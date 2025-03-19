#include <chrono>
#include <memory>

#include "Server.hpp"
#include "TutorialGame.h"

#include "Multiplayer/GamePackets.hpp"
#include "Multiplayer/GamePacketHandlers.hpp"
#include "Multiplayer/Lobby.hpp"
#include "Multiplayer/User.hpp"
#include "Multiplayer/WorldState.hpp"


namespace Multiplayer {
    using namespace WorldState;

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

    bool Server::IsOwnerOf(GameObject* obj) {
        return obj->GetOwner() == *m_user;
    }

    void Server::SendState(bool endOfTick) {
        if (endOfTick) return;
        
        if (m_isHost && m_game->GetState() == GameState::STARTING) {
            std::shared_ptr<Packet::StartGamePacket> startGame = std::make_shared<Packet::StartGamePacket>();
            m_network->Broadcast(startGame);
            m_game->SetState(GameState::ACTIVE);
        }

        if (m_game->GetState() != GameState::ACTIVE) return;

        m_game->GetWorld()->OperateOnContents([&](GameObject* object) {
            if (object->isStatic() && object->getType() != GameObject::Type::Gun) return;

            object->GetObjectStates()->UpdateBuffer();

            if (object->GetOwner() != *m_user) return;

            StateReader readReader = object->GetObjectStates()->GetReadState();
            ObjectState* read = readReader.GetState();

            if (read->Size() <= 0) return;

            StateValue linearVelocity;
            StateValue angularVelocity;
            StateValue position;
            StateValue rotation;

            read->Lock_Shared();

            bool hasLinear = read->ReadState(StateType::LinearVelocity, &linearVelocity);
            bool hasAngular = read->ReadState(StateType::AngularVelocity, &angularVelocity);

            bool hasPosition = read->ReadState(StateType::Position, &position);
            bool hasRotation = read->ReadState(StateType::Rotation, &rotation);

            read->Unlock_Shared();

            if (hasLinear && hasAngular) {
                std::shared_ptr<Packet::DeltaPacket> deltaPacket = std::make_shared<Packet::DeltaPacket>(
                    object->GetWorldID(),
                    std::get<btVector3>(linearVelocity),
                    std::get<btVector3>(angularVelocity),
                    m_tickCount
                );
                m_network->Broadcast(deltaPacket);
            }

            if (hasPosition && hasRotation) {
                std::shared_ptr<Packet::PositionPacket> positionPacket = std::make_shared<Packet::PositionPacket>(
                    object->GetWorldID(),
                    std::get<btVector3>(position),
                    std::get<btQuaternion>(rotation),
                    m_tickCount
                );
                m_network->Broadcast(positionPacket);
            }

            // Create change gravity packet for players.
            if (object->getType() == GameObject::Type::Player) {
                StateValue upVectorValue;

                read->Lock_Shared();
                bool hasUpVector = read->ReadState(StateType::UpVector, &upVectorValue);
                read->Unlock_Shared();

                std::shared_ptr<Packet::ObjectChangeGravityPacket> gravity = std::make_shared<Packet::ObjectChangeGravityPacket>(
                    object->GetWorldID(),
                    std::get<btVector3>(upVectorValue),
                    m_tickCount
                );
                m_network->Broadcast(gravity);
            }
            readReader.Unlock();
        });
    }

    void Server::ProcessPackets(bool endOfTick) {
        if (!endOfTick) return;

        // TODO: place packets into a buffer to add a little delay before processing so that
        // enough time has passed for all the packets to arrive.
        std::shared_ptr<Packet::Packet> currentPacket = m_network->Fetch();
        int smallestIncoming = INT32_MAX;

        while (currentPacket.get() != nullptr) {
            // Process packets that have a sequence of zero (usually high priority).
            if (currentPacket->GetChannel() == (uint8_t) Channel::RELIABLE ) {
                Packet::PacketRegister::GetHandler(currentPacket->GetType())->Handle(currentPacket);
            }

            // Add packet to the buffer.
            else {
                // Drop old packets.
                if (currentPacket->GetSequenceNumber() >= m_processTick) {
                    m_buffer[m_tickCount % TICK_BUFFER_SIZE].push_back(currentPacket);

                    if (currentPacket->GetSequenceNumber() < smallestIncoming) {
                        smallestIncoming = currentPacket->GetSequenceNumber();
                    }
                }

                // Pass packets on to clients.
                if (m_isHost) {
                    currentPacket->SetSequenceNumber(currentPacket->GetSequenceNumber() + 1);
                    m_network->Broadcast(currentPacket);
                }
            }

            currentPacket = m_network->Fetch();
        }

        // Reading.
        if (m_processTick >= 0) {
            for (std::shared_ptr<Packet::Packet> packet : m_buffer[m_processTick % TICK_BUFFER_SIZE]) {
                Packet::PacketRegister::GetHandler(packet->GetType())->Handle(packet);
            }
            m_buffer[m_processTick % TICK_BUFFER_SIZE].clear();
        }

        // Speeding up to match others pace.
        if (smallestIncoming > m_tickCount && smallestIncoming != INT32_MAX) {
            m_processTick += smallestIncoming - m_tickCount;
            m_tickCount = smallestIncoming;
        }

        m_tickCount++;
        m_processTick++;
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


