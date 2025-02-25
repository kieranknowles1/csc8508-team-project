#pragma once

#include "Network/Packet.hpp"


namespace Packet {
	const Type DELTA = CUSTOM_TYPE;
	const Type POSITION = CUSTOM_TYPE + 1;


	/**
	 * @brief Delta Packet class.
	 *
	 * Stores the velocity of an object.
	 */
	class DeltaPacket : public Packet {
		DeltaPacket() : m_type(DELTA) {
			
		}

	};

}

