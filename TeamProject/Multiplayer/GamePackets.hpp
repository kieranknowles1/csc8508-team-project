#pragma once

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

#include "Network/Network.hpp"
#include "PlayerObject.h"


namespace Packet {
	/**
	 * @brief PacketType Enum.
	 * 
	 * Contains user defined enums for the types of packets in the network.
	 */
	enum class PacketType : Type {
		DELTA = CUSTOM_TYPE,
		POSITION = CUSTOM_TYPE + 1,
		PLAYER_STATE_CHANGE = CUSTOM_TYPE + 2,
		OBJECT_CHANGE_GRAVITY = CUSTOM_TYPE + 3
	};


	/**
	 * @brief Delta Packet class.
	 *
	 * Stores the linear and angular velocity of an object.
	 * Used by clients for prediction of object positions.
	 */
	class DeltaPacket : public Packet {
	public:
		DeltaPacket(int objectID, const btVector3& linear, const btVector3& angular, int sequenceNum) :
			Packet(static_cast<Type>(PacketType::DELTA), static_cast<int>(Channel::FREQUENT), sequenceNum),
			m_objectID(objectID), m_linearVelocity(linear), m_angularVelocity(angular)
		{}

		/**
		 * @brief Get the ID of the object this delta packet should be applied to.
		 * @return int representing the GameObject's worldID.
		 */
		inline int GetTargetID() const { return m_objectID; }

		/**
		 * @brief Get the Linear Velocity of the Delta Packet.
		 * 
		 * This is the current linear velocity of the object on the server.
		 * 
		 * @return btVector3 containing the linear velocity.
		 */
		inline btVector3 GetLinearVelocity() const { return m_linearVelocity; }

		/**
		 * @brief Get the Angular Velocity of the Delta Packet.
		 * 
		 * This is the current angular velocity of thet object on the server.
		 * 
		 * @return btVector3 containing the angular velocity.
		 */
		inline btVector3 GetAngularVelocity() const { return m_angularVelocity; }

	private:
		const int m_objectID;
		const btVector3 m_linearVelocity;
		const btVector3 m_angularVelocity;
	};


	/**
	 * @brief Position Packet class.
	 * 
	 * Stores the position and orientation of the object.
	 */
	class PositionPacket : public Packet {
	public:
		PositionPacket(int objectID, const btVector3& position, const btQuaternion& orientation, int sequenceNum) :
			Packet(static_cast<Type>(PacketType::POSITION), static_cast<int>(Channel::FREQUENT), sequenceNum),
			m_objectID(objectID), m_position(position), m_orientation(orientation)
		{}
		
		/**
		 * @brief Get the ID of the object this position packet should be applied to.
		 * @return int representing the GameObject's worldID.
		 */
		inline int GetTargetID() const { return m_objectID; }

		/**
		 * @brief Get the new position of the game object from the packet.
		 * 
		 * @return btVector3 containing the new position data.
		 */
		inline btVector3 GetPosition() const { return m_position; }

		/**
		 * @brief Get the new orientation of the game object from the packet.
		 * 
		 * @return btQaternion containing the new orientation data. 
		 */
		inline btQuaternion GetOrientation() const { return m_orientation; }

	private:
		const int m_objectID;
		const btVector3 m_position;
		const btQuaternion m_orientation;
	};


	/**
	 * @brief Player Change State Packet class.
	 * 
	 * Used to notify of a player's state change (usually spawning in or
	 * dying).
	 */
	class PlayerChangeStatePacket : public Packet {
	public:
		PlayerChangeStatePacket(int playerID, const PlayerState& state, int sequenceNum) :
			Packet(static_cast<Type>(PacketType::PLAYER_STATE_CHANGE), static_cast<int>(Channel::UNSEQUENCED), sequenceNum),
			m_playerID(playerID), m_newState(state)
		{}

	private:
		int m_playerID;
		PlayerState m_newState;
	};


	/**
	 * @brief Objet Change Gravity Packet class.
	 * 
	 * Used to change the direction of an objects gravity.
	 * Changes the up vector of the object.
	 */
	class ObjectChangeGravity : public Packet {
	public:
		ObjectChangeGravity(int objectID, const btVector3& upDirection, int sequenceNum) :
			Packet(static_cast<Type>(PacketType::OBJECT_CHANGE_GRAVITY), static_cast<int>(Channel::UNSEQUENCED), sequenceNum),
			m_objectID(objectID), m_upVector(upDirection)
		{}

	private:
		int m_objectID;
		btVector3 m_upVector;
	};
}

