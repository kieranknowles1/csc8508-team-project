#pragma once

#include "Network/Packet.hpp"


namespace Packet {
	const Type DELTA = 1;



	/**
	 * @brief Delta Packet class.
	 *
	 * Stores the velocity of an object.
	 */
	class DeltaPacket : public Packet {
		DeltaPacket() {

		}

	};

}

