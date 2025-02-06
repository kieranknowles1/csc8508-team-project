#pragma once

#include <./enet/enet.h>

#include "Network.hpp"
#include <cstdint>
#include <climits>
#include <memory>

namespace Packet {
	typedef uint16_t Type;
	typedef unsigned short uint16_t;

	const Type NONE = 0;
	const unsigned short MAX_PACKET_SIZE = USHRT_MAX;

	/**
	 * @brief Stores the data for the packet.
	 */
	struct PacketBase {
		Type type;
		unsigned short size;
		char channel;
		std::shared_ptr<char[]> data;

		/**
		 * @brief Converts ENetEvent.packet.data into PacketBase.
		 * @param event The ENetEvent containing the packet to convert.
		 * @return A PacketBase variant of the ENetPacket
		 */
		static PacketBase FromENetEvent(ENetEvent* event);

		/**
		 * @brief Creates an ENetPacket from the data.
		 * @return A pointer to an enet packet.
		 */
		ENetPacket* ToENetPacket();
	};


	/**
	 * @brief Class for resolving packets.
	 * 
	 * Contains a single, overridable Handle function.
	 */
	class PacketHandler {
	public:
		virtual void Handle(PacketBase packet) = 0;
	};





}


