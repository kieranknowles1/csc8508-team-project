#pragma once

#include <Network/Network.hpp>

#include "Multiplayer/Lobby.hpp"

namespace Multiplayer {
    using namespace Lobbies;

    constexpr ENetAddress hostAddress = { ENET_HOST_ANY, DEFAULT_PORT };
    const int TICK_BUFFER_SIZE = 5;

    /**
     * @brief Server used to communicate with other players.
     * Handles the sending and receiving of game state.
     */
    class Server {
    public:
        /**
         * @brief If is the host, creates a server, otherwise, joins one.
         * @param isHost - Whether the server is the host or not.
         */
        Server(bool isHost = false);
        ~Server();

    private:
        /**
         * @brief Listens to the server tick and sends state when server
         * tick calls this function at the start of the tick.
         */
        void SendState(bool endOfTick);

        /**
         * @brief Listens to the server tick and processes packets when
         * the tick calls this function at the end of the tick.
         */
        void ProcessPackets(bool endOfTick);


        Lobby* m_lobby = nullptr;
        Network* m_network = nullptr;
        uint32_t m_tickCount = 0;
        bool m_isHost = false;
    };
}
