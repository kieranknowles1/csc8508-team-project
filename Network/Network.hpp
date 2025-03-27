#pragma once

#include <algorithm>
#include <array>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <cmath>

#include <./enet/enet.h>

#include "Packet.hpp"


// The rate at which packets will be sent and received.
const float NETWORK_RATE = 1 / 120.0f; // Seconds

// How many packets to store before denying packets.
const int BUFFER_SIZE = 1024;

// Default binding port.
const int DEFAULT_PORT = 49835;


/**
 * @brief Character codes for console colors.
 */
namespace ConsoleTextColor {
    inline const char* RED = "\x1b[31m";
    inline const char* GREEN = "\x1b[32m";
    inline const char* YELLOW = "\x1b[33m";
    inline const char* DEFAULT = "\x1b[0m";
};


/**
 * @brief Channel enum class.
 *
 * Defines the different channels that the network can use.
 * @warning Do not assign values to the channels.
 *
 */
enum class Channel {
    RELIABLE = 0,	// MUST BE FIRST CHANNEL AS FIRST CHANNEL IS ALWAYS RELIABLE.
    UNSEQUENCED,	// Used for packet data that cannot be lost but order is not important.
    FREQUENT,		// Used for packet data that can be lost and is frequently sent.
    CHANNEL_COUNT	// Used to count how many channels should be created.
};


/**
 * @brief Different states the network can be in.
 */
namespace NetworkStates {
    enum class NetworkState {
        CLOSED,
        OFF,
        ERRORED,
        ON
    };
}


class Network {
public:
    Network(const ENetAddress* address, int maxConnections);
    ~Network();

    /**
     * @brief Start the thread for receiving and sending packets.
     *
     * Starts a new thread. The thread handles sending and receiving of packets.
     */
    void Start();

    /**
     * @brief Stop and join the execution thread.
     *
     * Sets network state to OFF
     */
    void Stop();

    /**
     * @brief Closes the host.
     *
     * Cleans up enet objects. Calls Stop() first.
     */
    void Close();

    /**
     * @brief Connect to another ENetHost.
     * @param destination The address of the endpoint to connect to.
     */
    void ConnectTo(const ENetAddress* destination);

    /**
     * @brief Queue a Packet to be send next tick via Broadcasting.
     * Is threadsafe.
     * @param packet The packet to send.
     */
    void Broadcast(std::shared_ptr<Packet::Packet> packet);


    /**
     * @brief Send a Packet to a specific Peer.
     * Is threadsafe.
     * @param packet - The packet to send
     * @param destination - The Peer to send the packet to.
     */
    void Send(std::shared_ptr<Packet::Packet> packet, ENetPeer* destination);


    /**
     * @brief Assign which function to call upon receiving a connect packet.
     *
     * Passes ENetPeer* incase the callback function requires it.
     *
     * @param callback The function to call.
     */
    inline void SetConnectCallback(std::function<void(ENetPeer*)> callback) {
        std::lock_guard<std::mutex> lock(m_connectCallbackMut);
        m_connectCallback = callback;
    }

    /**
     * @brief Fetch a packet from the buffer.
     *
     * Fetch from the buffer. The buffer is updated every tick. Is threadsafe.
     *
     * @return The next packet in the buffer. Empty packet if no packet exists.
     */
    std::shared_ptr<Packet::Packet> Fetch();

    /**
     * @brief Get the current state of the network.
     * Is Threadsafe.
     * @return
     */
    NetworkStates::NetworkState GetState() {
        std::lock_guard<std::mutex> lock(m_stateMut);
        return m_state;
    }

    /**
     * @brief Get the number of external connections.
     * @return int representing the number of external connections.
     */
    int GetConnectionCount() const { return m_connections; }

    /**
     * @brief Add a function to be called when the server Ticks.
     * The function is called before SendAll() is called.
     *
     * PLEASE ENSURE THE FUNCTION CALL IS THREADSAFE. IT IS CALLED FROM WITHIN
     * THE NETWORK THREAD.
     *
     * If the value passed to the function is true, it is the end of the tick.
     *
     * @param func - the function to call.
     */
    inline void AddTickListener(std::function<void(bool)> func) {
        std::lock_guard<std::mutex> lock(m_listenerMut);
        m_tickListeners.push_back(func);
    }

    /**
     * @brief Return the amount of time that has elapsed since the last tick
     * represented as a value betweeen 0 and 1.
     */
    inline float GetTickProgress() {
        std::shared_lock lock(m_tickProgressMut);
        return std::fmod(m_lastTime, NETWORK_RATE) / NETWORK_RATE;
    }

    std::unique_lock<std::mutex> LockTick() {
        return std::move(std::unique_lock(m_tickLock));
    }


protected:
    /**
     * @brief The entry point for the thread for sending and receiving packets.
     */
    void Run();

    /**
     * @brief Step forward by dt.
     *
     * If enough time has elapsed the server will fetch and send packets based
     * on the NETWORK_RATE.
     */
    void Tick(float dt);

    /**
     * @brief Sends all queued packets.
     */
    void SendAll();

    /**
     * @brief Set the network into an errored state.
     */
    void SetErrored();

private:
    bool ConnectPeer();
    void HandleIncomingPacket(ENetEvent* event);
    void DisconnectPeer();

    std::thread m_networkThread;
    std::mutex m_stateMut;
    std::mutex m_sendMut;
    std::mutex m_connectCallbackMut;
    std::mutex m_listenerMut;
    std::mutex m_tickLock;
    std::shared_mutex m_tickProgressMut;

    NetworkStates::NetworkState m_state = NetworkStates::NetworkState::CLOSED;

    Packet::PacketBuffer m_receiveBuffer = Packet::PacketBuffer(BUFFER_SIZE);
    std::vector<std::pair<std::shared_ptr<Packet::Packet>, ENetPeer*>> m_sendBuffer = std::vector<std::pair<std::shared_ptr<Packet::Packet>, ENetPeer*>>(BUFFER_SIZE);
    int m_numPackets = 0;

    float m_elapsedTime = 0;
    float m_lastTime = 0;
    float m_lastTick = 0;
    int m_lastMaxSequence = 0; // Each tick will drop optional packets that didn't make the first tick.

    int m_connections = 0;
    int m_maxConnections;

    ENetHost* m_host = nullptr;
    std::function<void(ENetPeer*)> m_connectCallback;
    std::vector<std::function<void(bool)>> m_tickListeners;
};
