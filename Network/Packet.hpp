#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include <./enet/enet.h>

#include "Network.hpp"

namespace Packet {
	typedef uint16_t Type;
	typedef unsigned short uint16_t;

	const uint8_t REQUIRED_CHANNEL = 0;


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
		Packet() {}

		bool operator<(const Packet& other) const { return m_sequenceNum < other.m_sequenceNum; }

	private:
		Type m_type;
		uint8_t m_channel;
		uint32_t m_sequenceNum;
	};


	/**
	 * @brief PacketHandler Class.
	 */
	class PacketHandler {
	public:
		PacketHandler() {};

		/**
		 * @brief Handle the packet.
		 * @param packet The ENetPacket that has been received.
		 * 
		 * Deserializes the packet and then processes it.
		 */
		virtual void Handle(Packet packet) = 0;

		/**
		 * @brief Convert the ENet packet into a Packet type.
		 * @param packet the received ENetPacket.
		 * @return Pointer to the created packet.
		 */
		virtual Packet Translate(ENetPacket* packet) = 0;

		/**
		 * @brief Creates an ENet Packet.
		 * @return A new ENetPacket. Please manage accordinly.
		 */
		virtual ENetPacket* ToENetPacket(Packet packet) = 0;

		/**
		 * @brief Get the type of the packet this object handles.
		 * @return Type of the packet.
		 */
		Type GetType() const { return type; }

		/**
		 * @brief Compare 2 PacketHandlers.
		 * @param other The other packet handler.
		 * @return True if both PacketHandlers handle the same packet type.
		 */
		bool operator==(const PacketHandler& other) const {
			return type == other.GetType();
		}

	private:
		Type type;
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

	private:
		PacketRegister() { if (packetRegister == nullptr) packetRegister = this; }

		static inline PacketRegister* packetRegister = nullptr;
		static std::unordered_map<Type, PacketHandler*, TypeHash> handlers;
	};


	class PacketBuffer {
	public:
		PacketBuffer() {}

		/**
		 * @brief Insert a packet into the buffer.
		 * @param item A buffer item.
		 * 
		 * Inserts a packet into the buffer unless the buffer is full in which case
		 * the packet will not be added..
		 */
		bool Insert(Packet item) {
			// Drop packets when buffer is full.
			if (!IsFull()) {
				m_packets[m_numPackets++] = item;
				std::push_heap(m_packets.begin(), m_packets.end());
				return true;
			}
			return false;
		}


		/**
		 * @brief Fetch the item at the head of the queue.
		 * @return The buffer item in the queue.
		 */
		Packet Pop() {
			if (IsEmpty()) return Packet();

			std::pop_heap(m_packets.begin(), m_packets.end());
			return m_packets[--m_numPackets];
		}

		/**
		 * @brief Get the item that is at the head of the queue without removing.
		 * @return The item at the front of the queue.
		 */
		const Packet Peek() const {
			if (IsEmpty()) return Packet();
			return m_packets[0];
		}

		bool IsEmpty() const { return m_numPackets <= 0; }
		bool IsFull() const { return m_numPackets >= BUFFER_SIZE; }

	private:
		int m_numPackets = 0;
		std::array<Packet, BUFFER_SIZE> m_packets;
	};


}


