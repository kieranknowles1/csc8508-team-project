#pragma once

#include "TutorialGame.h"
#include <Network/Network.hpp>

namespace Lobbies {
    class User;
    class Lobby;
}



namespace Multiplayer {

    constexpr ENetAddress hostAddress = { ENET_HOST_ANY, DEFAULT_PORT };
    const int TICK_BUFFER_SIZE = 5;

    /**
     * @brief Server used to communicate with other players.
     * Handles the sending and receiving of game state.
     */
    class Server {
    public:
        /**
         * @brief Starts the network and creates a lobby and a unique user.
         * @param isHost - Whether the server is the host or not.
         */
        Server(TutorialGame* game, bool isHost = false);
        ~Server();

        /**
         * @brief Register all necessary packet handlers.
         * Packet register is static so this only needs to be called once.
         */
        void InitPacketHandlers();

        /**
         * @brief Connect and busy-wait for the server to respond.
         * @param ip - String representation of the ip (XX.XX.XX.XX).
         * @param waitSeconds - How many seconds to wait before giving up.
         */
        void JoinGame(const std::string& ip, float waitSeconds);

        bool IsConnected() const { return m_connected; }
        bool IsHost() const { return m_isHost; }

    private:
        /**
         * @brief Listens to the server tick and sends state when server
         * tick calls this function at the start of the tick.
         * 
         * MUST BE THREAD SAFE AS IT IS CALLED FROM THE NETWORK THREAD.
         */
        void SendState(bool endOfTick);

        /**
         * @brief Listens to the server tick and processes packets when
         * the tick calls this function at the end of the tick.
         * 
         * MUST BE THREAD SAFE AS IT IS CALLED FROM THE NETWORK THREAD.
         */
        void ProcessPackets(bool endOfTick);

        /**
         * @brief Listens for connections to the server and sends relevant
         * information to the connecting user. This is only used by the host.
         */
        void OnClientJoin(ENetPeer* peer);


        TutorialGame* m_game = nullptr;
        Network* m_network = nullptr;

        Lobbies::User* m_user = nullptr;
        Lobbies::Lobby* m_lobby = nullptr;

        std::vector<std::unique_ptr<Packet::PacketHandler>> m_handlers;

        uint32_t m_tickCount = 0;
        bool m_isHost = false;
        bool m_connected = false;
        unsigned int m_uniqueUserID = 0;
    };
}
