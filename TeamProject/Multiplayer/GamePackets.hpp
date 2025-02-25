#pragma once

#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btQuaternion.h>

#include "Network/Network.hpp"
#include "PlayerObject.h"


namespace Packet {
	const Type DELTA = CUSTOM_TYPE;
	const Type POSITION = CUSTOM_TYPE + 1;
	const Type PLAYER_DEATH = CUSTOM_TYPE + 2;
	const Type PLAYER_SPAWN = CUSTOM_TYPE + 3;
	const Type PLAYER_GRAVITY_CHANGE = CUSTOM_TYPE + 4;


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


	/**
	 * @brief Position Packet class.
	 * 
	 * Stores the position and orientation of the object.
	 */
	class PositionPacket : public Packet {
	public:
		PositionPacket(int objectID, const btVector3& position, const btQuaternion& orientation, int sequenceNum) :
			Packet(DELTA, static_cast<int>(Channel::FREQUENT), sequenceNum),
			m_objectID(objectID), m_position(position), m_orientation(orientation)
		{}

	private:
		int m_objectID;
		btVector3 m_position;
		btQuaternion m_orientation;
	};


	/**
	 * @brief Player Change State Packet class.
	 * 
	 * Used to notify of a player's state change (usually spawning in or
	 * dying).
	 */
	class PlayerChangeStatePacket : public Packet {
	public:
		PlayerChangeStatePacket(int objectID, int playerID, PlayerState state, int sequenceNum) :
			Packet(DELTA, static_cast<int>(Channel::UNSEQUENCED), sequenceNum),
			m_objectID(objectID), m_playerID(playerID), m_newState(state)
		{}

	private:
		int m_objectID;
		int m_playerID;
		PlayerState m_newState;

	};
}

