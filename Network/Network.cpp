#include <iostream>
#include <chrono>

#include <cstring>

#include "Network.hpp"

Network::Network(const ENetAddress* address, int maxConnections) : m_maxConnections(maxConnections) {
    if (enet_initialize() != 0) {
        SetErrored();
        return;
    }

    m_host = enet_host_create(address, maxConnections, static_cast<int>(Channel::CHANNEL_COUNT), 0, 0);
    if (m_host == nullptr) {
        SetErrored();
        return;
    }

    std::lock_guard<std::mutex> lock(m_stateMut);
    m_state = NetworkState::OFF;
}


Network::~Network() {
    if (GetState() != NetworkState::CLOSED) {
        Close();
    }
}


void Network::Start() {
    std::lock_guard<std::mutex> lock(m_stateMut);
    if (m_state != NetworkState::OFF) return;

    m_state = NetworkState::ON;
    m_networkThread = std::thread(&Network::Run, this);
}


void Network::Stop() {
    m_stateMut.lock();
    if (!(m_state == NetworkState::ON)) {
        m_stateMut.unlock();
        return;
    }
    m_state = NetworkState::OFF;
    m_stateMut.unlock();
    m_networkThread.join();
}


void Network::Close() {
    Stop();

    enet_host_destroy(m_host);
    enet_deinitialize();
    m_connections = 1;

    std::lock_guard<std::mutex> lock(m_stateMut);
    m_state = NetworkState::CLOSED;
}


void Network::ConnectTo(const ENetAddress* destination) {
    ENetPeer* peer = enet_host_connect(m_host, destination, static_cast<int>(Channel::CHANNEL_COUNT), 0);
}


void Network::Broadcast(std::shared_ptr<Packet::Packet> packet) {
    std::lock_guard<std::mutex> lock(m_sendMut);
    m_sendBuffer[m_numPackets++] = std::pair(packet, nullptr);
}


void Network::Send(std::shared_ptr<Packet::Packet> packet, ENetPeer* peer) {
    std::lock_guard<std::mutex> lock(m_sendMut);
    m_sendBuffer[m_numPackets++] = std::pair(packet, peer);
}


std::shared_ptr<Packet::Packet> Network::Fetch() {
    std::shared_ptr<Packet::Packet> fetched;

    while (!m_receiveBuffer.IsEmpty()) {
        fetched = m_receiveBuffer.Pop();
        return fetched;
    }
    return fetched; // Return an empty packet.
}


void Network::Run() {
    auto last = std::chrono::high_resolution_clock::now();
    auto now = last;
    std::chrono::duration<float> dt;

    bool running = true;

    while (running) {
        if (GetState() != NetworkState::ON) {
            running = false;
            return;
        }

        now = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last);
        last = now;

        Tick(dt.count());
    }
}


void Network::Tick(float dt) {
    m_elapsedTime += dt;

    while (m_elapsedTime - m_lastTick >= NETWORK_RATE) {
        m_lastTick += NETWORK_RATE;

        SendAll();
        enet_host_flush(m_host);
        ENetEvent event;

        while (enet_host_service(m_host, &event, 10) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                if (!ConnectPeer()) {
                    enet_peer_disconnect(event.peer, 0);
                }
                else if (m_connectCallback != nullptr) {
                    m_connectCallback(event.peer);
                }
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                DisconnectPeer();
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                HandleIncomingPacket(&event);
                break;
            }
            enet_packet_destroy(event.packet);
        }
    }
}


void Network::SendAll() {
    Packet::PacketRegister* packetRegister = Packet::PacketRegister::GetRegister();

    for (int i = 0; i < m_numPackets; i++) {
        Packet::PacketHandler* handler = packetRegister->GetHandler(m_sendBuffer[i].first.get()->GetType());
        
        ENetPacket* packet = handler->ToENetPacket(m_sendBuffer[i].first);

        if (m_sendBuffer[i].second == nullptr) {
            enet_host_broadcast(m_host, m_sendBuffer[i].first.get()->GetChannel(), packet);
        }
        else {
            enet_peer_send(m_sendBuffer[i].second, m_sendBuffer[i].first.get()->GetChannel(), packet);
        }
    }
    m_numPackets = 0;
}


void Network::SetErrored() {
    std::lock_guard<std::mutex> lock(m_stateMut);
    m_state = NetworkState::ERRORED;
}


bool Network::ConnectPeer() {
    if (m_connections < m_maxConnections) {
        m_connections++;
        return true;
    }
    return false;
}


void Network::HandleIncomingPacket(ENetEvent* event) {
    ENetPacket* packet = event->packet;
    Packet::Type packetType;
    memcpy(&packetType, packet->data, sizeof(Packet::Type));

    Packet::PacketRegister* packetRegister = Packet::PacketRegister::GetRegister();
    Packet::PacketHandler* packetHandler = packetRegister->GetHandler(packetType);
    std::shared_ptr<Packet::Packet> translated = packetHandler->Translate(event);

    m_receiveBuffer.Insert(translated);
}


void Network::DisconnectPeer() {
    m_connections--;
    if (m_connections < 0) SetErrored();
}
