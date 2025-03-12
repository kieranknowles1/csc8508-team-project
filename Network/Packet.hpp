#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <thread>
#include <mutex>

#include <./enet/enet.h>

namespace Packet {
    typedef uint16_t Type;
    typedef unsigned short uint16_t;

    const Type NONE = 0;
    const uint8_t REQUIRED_CHANNEL = 0;

    const Type CUSTOM_TYPE = 64;

    /**
     * @brief Packet class.
     * 
     * Stores basic information about the packet including:
     * The type of packet.
     * The channel it is to be sent on.
     * The Sequence number of the packet.
     */
    class Packet {
    public:
        Packet() : m_type(NONE), m_channel(0), m_sequenceNum(0) {}
        Packet(Type type, uint8_t channel, uint32_t sequenceNum) : m_type(type), m_channel(channel), m_sequenceNum(sequenceNum) {}

        /**
         * @brief Get the type of the packet.
         * @return Type of the packet.
         */
        Type GetType() { return m_type; }

        /**
         * @brief Get the channel this packet uses.
         * @return The channel number the packet uses.
         */
        uint8_t GetChannel() { return m_channel; }

        /**
         * @brief Get the sequence number of the packet.
         * @return Returns the sequence number of the packet.
         */
        uint32_t GetSequenceNumber() { return m_sequenceNum; }

        /**
         * @brief Override of the < operator for heap ordering.
         * @param other The packet the compare to.
         * @return True if the sequence number is MORE THAN the others.
         * 
         * Reverse less than operator so that packets with a lower sequence
         * number have a higher priority.
         */
        bool operator<(const Packet& other) const { return m_sequenceNum > other.m_sequenceNum; }

    protected:
        Type m_type;
        uint8_t m_channel;
        uint32_t m_sequenceNum;
    };


    /**
     * @brief PacketHandler Class.
     */
    class PacketHandler {
    public:
        PacketHandler(Type type) : m_type(type) {};

        /**
         * @brief Handle the packet.
         * @param packet The ENetPacket that has been received.
         * 
         * Deserializes the packet and then processes it.
         */
        virtual void Handle(const std::shared_ptr<Packet> packet) = 0;

        /**
         * @brief Convert the ENet packet into a Packet type.
         * @param event - The received ENetEvent containing the packet.
         * @return Pointer to the translated packet.
         */
        virtual std::shared_ptr<Packet> Translate(const ENetEvent* event) const = 0;

        /**
         * @brief Creates an ENet Packet.
         * @return A new ENetPacket.
         * @warning Remember to call enet_packet_destroy IF
         * you do not send the packet.
         */
        virtual ENetPacket* ToENetPacket(const std::shared_ptr<Packet> packet) const = 0;

        /**
         * @brief Get the type of the packet this object handles.
         * @return Type of the packet.
         */
        Type GetType() const { return m_type; }

        /**
         * @brief Compare 2 PacketHandlers.
         * @param other The other packet handler.
         * @return True if both PacketHandlers handle the same packet type.
         */
        bool operator==(const PacketHandler& other) const {
            return m_type == other.GetType();
        }

    protected:
        void GetBaseData(const ENetPacket* packet, Type* type, uint8_t* channel, uint32_t* sequenceNum) const {
            memcpy(type, packet->data, sizeof(Type));
            memcpy(channel, packet->data + sizeof(Type), sizeof(uint8_t));
            memcpy(sequenceNum, packet->data + sizeof(Type) + sizeof(uint8_t), sizeof(uint32_t));
        }

    private:
        Type m_type = 0;
    };


    /**
     * @brief Hashing object for Packet Handlers.
     */
    struct TypeHash {
        std::size_t operator()(Type type) const {
            return static_cast<size_t>(type);
        }
    };


    /**
     * @brief PacketRegister class.
     * 
     * Stores a bunch of packet handlers for handling incoming packets.
     */
    class PacketRegister {
    public:
        /**
         * @brief Fetch the PacketRegister.
         * @return The PacketRegister.
         */
        static PacketRegister* GetRegister() { 
            if (packetRegister == nullptr) PacketRegister();
            return packetRegister;
        }

        /**
         * @brief Register a packet handler with the register.
         * @param handler The handler for the packet.
         */
        static void Register(PacketHandler* handler) { handlers[handler->GetType()] = handler; }

        /**
         * @brief Get the handler for the requested packet type.
         * @param type The type of the packet.
         */
        static PacketHandler* GetHandler(Type type) {
            if (handlers.contains(type)) {
                return handlers[type];
            }
            return nullptr;
        }

    private:
        PacketRegister() { if (packetRegister == nullptr) packetRegister = this; }

        static inline PacketRegister* packetRegister = nullptr;
        static inline std::unordered_map<Type, PacketHandler*, TypeHash> handlers;
    };


    /**
     * @brief PacketBuffer class.
     * 
     * A threadsafe buffer for storing packets until they are processed.
     */
    class PacketBuffer {
    public:
        PacketBuffer(int size) : m_size(size) { m_packets = std::make_unique<std::shared_ptr<Packet>[]>(size); }

        /**
         * @brief Insert a packet into the buffer.
         * @param item A buffer item.
         *
         * Inserts a packet into the buffer unless the buffer is full in which case
         * the packet will not be added..
         */
        bool Insert(std::shared_ptr<Packet> item) {
            // Drop packets when buffer is full.
            if (!IsFull()) {

                std::lock_guard<std::mutex> bufferLock(m_bufferMut);
                std::lock_guard<std::mutex> totalLock(m_totalMut);

                m_packets.get()[m_numPackets++] = item;
                std::push_heap(Begin(), Begin() + m_numPackets);
                return true;
            }
            return false;
        }


        /**
         * @brief Fetch the item at the head of the queue.
         * @return The buffer item in the queue.
         */
        std::shared_ptr<Packet> Pop() {
            if (IsEmpty()) return std::shared_ptr<Packet>(nullptr);

            std::lock_guard<std::mutex> bufferLock(m_bufferMut);
            std::lock_guard<std::mutex> totalLock(m_totalMut);

            std::pop_heap(Begin(), Begin() + m_numPackets);
            return m_packets.get()[--m_numPackets];
        }

        /**
         * @brief Get the item that is at the head of the queue without removing.
         * @return The item at the front of the queue or an empty packet if the
         *			buffer is empty.
         */
        inline std::shared_ptr<Packet> Peek() {
            if (IsEmpty()) return std::make_shared<Packet>();

            std::lock_guard<std::mutex> lock(m_bufferMut);
            return m_packets.get()[0];
        }

        /**
         * @brief Check if the buffer is empty.
         * @return True if the buffer is empty. Otherwise, false.
         */
        inline bool IsEmpty() {
            std::lock_guard<std::mutex> lock(m_totalMut);
            return m_numPackets <= 0;
        }

        /**
         * @brief Check if the buffer is full.
         * @return True if the buffer is full. Otherwise, false.
         */
        inline bool IsFull() {
            std::lock_guard<std::mutex> lock(m_totalMut);
            return m_numPackets >= m_size;
        }

    private:
        inline std::shared_ptr<Packet>* Begin() { return m_packets.get(); }
        inline std::shared_ptr<Packet>* End() { return m_packets.get() + m_size; }

        std::mutex m_totalMut;
        std::mutex m_bufferMut;

        int m_size;
        int m_numPackets = 0;
        std::unique_ptr<std::shared_ptr<Packet>[]> m_packets;
    };
}


