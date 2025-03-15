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
         */
        void InitPacketHandlers();

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

        TutorialGame* m_game = nullptr;
        Lobbies::User* m_user = nullptr;
        Lobbies::Lobby* m_lobby = nullptr;
        std::vector<std::unique_ptr<Packet::PacketHandler>> m_handlers;
        Network* m_network = nullptr;
        uint32_t m_tickCount = 0;
        bool m_isHost = false;
    };
}
