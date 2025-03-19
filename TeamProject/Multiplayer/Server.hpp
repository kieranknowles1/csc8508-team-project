#pragma once

#include "TutorialGame.h"
#include <Network/Network.hpp>
#include "Multiplayer/Lobby.hpp"

namespace Multiplayer {

    constexpr ENetAddress hostAddress = { ENET_HOST_ANY, DEFAULT_PORT };
    const int TICK_BUFFER_SIZE = 6;

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

        /**
         * @brief Used to create a user object for this network.
         * Frees the previous user if there was one.
         */
        void SetUser(const Lobbies::User& user);

        Lobbies::User* GetUser() const { return m_user; }

        void Start() { m_network->Start(); }

        /**
         * @brief Determine if the owner of the server is also the owner of the
         * game object.
         */
        bool IsOwnerOf(GameObject* object);

        inline void AddUserToLobby(const Lobbies::User& user) { m_lobby->AddUser(user); }
        inline void RemoveUserFromLobby(const Lobbies::User& user) { m_lobby->RemoveUser(user); }
        inline void AssignLobbyHost(const Lobbies::User& user) { m_lobby->SetHost(user); }

        inline bool IsConnected() const { return m_connected; }
        inline bool IsHost() const { return m_isHost; }
        inline unsigned int ClientCount() const { return m_lobby->GetConnectedUsers().size(); }
        inline unsigned int GetMaxClients() const { return MAX_PLAYERS; }

        /**
         * @brief Reset the tick to zero.
         * Used to synchronise the start of the game.
         */
        void ResetTick() {
            m_tickCount = 0;
            m_processTick = -TICK_BUFFER_SIZE + 1;
        }

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
        int m_processTick = -TICK_BUFFER_SIZE + 1;
        bool m_isHost = false;
        bool m_connected = false;
        unsigned int m_uniqueUserID = 0;

        std::array<std::vector<std::shared_ptr<Packet::Packet>>, TICK_BUFFER_SIZE> m_buffer;
    };
}
