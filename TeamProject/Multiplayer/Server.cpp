#include "Server.hpp"
#include "TutorialGame.h"
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
    }

    void Server::ProcessPackets(bool endOfTick) {
        if (!endOfTick) return;
    }





}


