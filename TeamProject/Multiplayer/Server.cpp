#include <chrono>
#include <memory>

#include "Health.h"
#include "Score.h"

#include "Server.hpp"
#include "TutorialGame.h"

#include "Multiplayer/GamePackets.hpp"
#include "Multiplayer/GamePacketHandlers.hpp"
#include "Multiplayer/Lobby.hpp"
#include "Multiplayer/User.hpp"
#include "WorldState.h"
#include "ServerObject.h"

using namespace Lobbies;

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

        m_handlers.push_back(std::make_unique<Packet::LaserPacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());

        m_handlers.push_back(std::make_unique<Packet::StartGamePacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());

        m_handlers.push_back(std::make_unique<Packet::ObjectChangeGravityPacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());

        m_handlers.push_back(std::make_unique<Packet::DamagePacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());

        m_handlers.push_back(std::make_unique<Packet::DeathPacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());

        m_handlers.push_back(std::make_unique<Packet::PlayerAnimationPacketHandler>());
        Packet::PacketRegister::Register(m_handlers.back().get());
    }

    void Server::JoinGame(const std::string& ip, float waitSeconds) {
        ENetAddress destination = { 0, DEFAULT_PORT };
        enet_address_set_host(&destination, ip.c_str());

        m_network->ConnectTo(&destination);

        int resendDelay = 10; // ms
        int maxAttempts = (waitSeconds * 1000 / resendDelay) + 1;
        int attempts = 0;

        do {
            std::this_thread::sleep_for(std::chrono::milliseconds(resendDelay));
            attempts++;
        } while (m_network->GetConnectionCount() < 1 && attempts < maxAttempts);

        m_connected = m_network->GetConnectionCount() > 0;
    }

    void Server::SetUser(const Lobbies::User& user) {
        if (m_user != nullptr) {
            delete m_user;
        }
        m_user = new Lobbies::User(user);
    }

    bool Server::IsOwnerOf(GameObject* obj) {
        return *(obj->GetOwner()) == *m_user;
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
            if (!object->IsNetworked()) return;
            if (object->GetOwner() == nullptr) return;
            if (*(object->GetOwner()) != *m_user) return;

            if (object->getType() == GameObject::Type::Player) {
                PlayerObject* player = (PlayerObject*)object;
                player->GetAttackAttrib()->GetWorldStates()->UpdateBuffer();
                player->GetHealthAttrib()->GetWorldStates()->UpdateBuffer();
                player->GetScoreAttrib()->GetWorldStates()->UpdateBuffer();
            }

            ServerObject* netObj = (ServerObject*)object;
            netObj->GetWorldStates()->UpdateBuffer();

            std::vector<std::shared_ptr<Packet::Packet>> packets = netObj->CreatePackets(m_tickCount);
            for (auto packet = packets.begin(); packet != packets.end(); packet++) {
                m_network->Broadcast(*packet);
            }
        });
    }

    void Server::ProcessPackets(bool endOfTick) {
        if (!endOfTick) return;

        std::shared_ptr<Packet::Packet> currentPacket = m_network->Fetch();
        int smallestIncoming = INT32_MAX;

        while (currentPacket.get() != nullptr) {
            // High Priority packets.
            if (currentPacket->GetSequenceNumber() == 0) {
                Packet::PacketRegister::GetHandler(currentPacket->GetType())->Handle(currentPacket);

                // Pass packets on to clients.
                if (m_isHost) {
                    m_network->Broadcast(currentPacket);
                }
            }

            // Add packet to the buffer.
            else {
                // Drop old packets.
                if (currentPacket->GetSequenceNumber() >= m_processTick) {
                    m_buffer[currentPacket->GetSequenceNumber() % TICK_BUFFER_SIZE].push_back(currentPacket);

                    if (currentPacket->GetSequenceNumber() < smallestIncoming) {
                        smallestIncoming = currentPacket->GetSequenceNumber();
                    }

                    // Pass packets on to clients.
                    if (m_isHost) {
                        // Add 1 to sequence number as this function is called at the end of a tick.
                        currentPacket->SetSequenceNumber(currentPacket->GetSequenceNumber() + 1);
                        m_network->Broadcast(currentPacket);
                    }
                }

                // Moving too fast.
                if (currentPacket->GetSequenceNumber() > m_tickCount) {
                    int diff = currentPacket->GetSequenceNumber() - m_tickCount;
#ifndef NDEBUG
                    std::cout << ConsoleTextColor::YELLOW;
                    std::cout << "Someone's network is ticking faster!\n";
                    std::cout << "Skipping " << diff << " ticks.\n";
                    std::cout << ConsoleTextColor::DEFAULT;
#endif
                    for (diff; diff > 0; diff--) {
                        if (m_processTick >= 0) m_buffer[m_processTick % TICK_BUFFER_SIZE].clear();
                        m_processTick++;
                    }
                    m_tickCount = currentPacket->GetSequenceNumber();
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

        // Swapping buffers after writing new states.
        m_game->GetWorld()->OperateOnContents([&](GameObject* object) {
            if (!object->IsNetworked()) return;

            ServerObject* netObj = (ServerObject*)object;

            if (netObj->GetOwner() == nullptr) return;
            if (*(netObj->GetOwner()) == *m_user) return;
            netObj->GetWorldStates()->UpdateBuffer();

            if (object->getType() == GameObject::Type::Player) {
                PlayerObject* player = (PlayerObject*)object;
                player->GetAttackAttrib()->GetWorldStates()->UpdateBuffer();
                player->GetHealthAttrib()->GetWorldStates()->UpdateBuffer();
                player->GetScoreAttrib()->GetWorldStates()->UpdateBuffer();
            }
            });

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

        // Send current lobby information.
        for (const User& player : m_lobby->GetConnectedUsers()) {
            std::shared_ptr<Packet::UserInfoPacket> playerInfo = std::make_shared<Packet::UserInfoPacket>(
                player,
                LobbyAction::JOIN
            );
            m_network->Send(playerInfo, client);
        }
        m_lobby->AddUser(clientUser);

        // Broadcast new member to the rest of the lobby.
        std::shared_ptr<Packet::UserInfoPacket> broadcast = std::make_shared<Packet::UserInfoPacket>(
            clientUser,
            LobbyAction::JOIN
        );
        m_network->Broadcast(broadcast);
    }
}

