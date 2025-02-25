#pragma once

#include "Network/Network.hpp"
#include "bullet/LinearMath/btVector3.h"


namespace Packet {
	const Type DELTA = CUSTOM_TYPE;
	const Type POSITION = CUSTOM_TYPE + 1;


	/**
	 * @brief Delta Packet class.
	 *
	 * Stores the linear and angular velocity of an object.
	 * Used by clients for prediction of object positions.
	 */
	class DeltaPacket : public Packet {
	public:
		DeltaPacket(int objectID, const btVector3& linear, const btVector3& angular, int sequenceNum) :
			Packet(DELTA, static_cast<int>(Channel::FREQUENT), sequenceNum),
			m_objectID(objectID), m_linearVelocity(linear), m_angularVelocity(angular)
		{}

	private:
		int m_objectID;
		btVector3 m_linearVelocity;
		btVector3 m_angularVelocity;
	};
}

