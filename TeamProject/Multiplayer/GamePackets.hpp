#pragma once

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

#include "Network/Network.hpp"
#include "PlayerObject.h"
#include "User.hpp"
#include "Lobby.hpp"

using namespace Lobbies;

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
        OBJECT_CHANGE_GRAVITY = CUSTOM_TYPE + 3,
        START_GAME = CUSTOM_TYPE + 4,
        USER_INFO = CUSTOM_TYPE + 5,
        ASSIGN_HOST = CUSTOM_TYPE + 6,
        REQUEST_USERID = CUSTOM_TYPE + 7
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
         * This is the current angular velocity of the object on the server.
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
        PlayerChangeStatePacket(int playerID, const PlayerState& state) :
            Packet(static_cast<Type>(PacketType::PLAYER_STATE_CHANGE), static_cast<int>(Channel::UNSEQUENCED), 0),
            m_playerID(playerID), m_newState(state)
        {}

        int GetPlayerID() const { return m_playerID; }
        PlayerState GetState() const { return m_newState; }

    private:
        int m_playerID;
        PlayerState m_newState;
    };


    /**
     * @brief A packet that updates an objects upVector to change its gravity.
     */
    class ObjectChangeGravityPacket : public Packet {
    public:
        ObjectChangeGravityPacket(int objectID, const btVector3& upDirection, uint32_t sequenceNum) :
            Packet(static_cast<Type>(PacketType::OBJECT_CHANGE_GRAVITY), static_cast<int>(Channel::RELIABLE), sequenceNum),
            m_objectID(objectID), m_upDirection(upDirection)
        {}

        int GetTargetID() const { return m_objectID; }
        btVector3 GetUpDirection() const { return m_upDirection; }

    private:
        int m_objectID;
        btVector3 m_upDirection;
    };

    /**
     * @brief A simple packet to be broadcast to all players simultaneously to
     * start the game.
     */
    class StartGamePacket : public Packet {
    public:
        StartGamePacket() :
            Packet(static_cast<Type>(PacketType::START_GAME), static_cast<uint8_t>(Channel::RELIABLE), 0)
        {}
    };


    /**
     * @brief A packet for assigning the host of a lobby.
     */
    class AssignHostPacket : public Packet {
    public:
        /**
         * @brief Constructor for AssignHostPacket.
         * @param hostID The ID of the host user.
         * @param destination - If nullptr, will broadcast.
         */
        AssignHostPacket(int hostID, ENetPeer* destination = nullptr) :
            Packet(static_cast<Type>(PacketType::ASSIGN_HOST), static_cast<uint8_t>(Channel::UNSEQUENCED), 0),
            m_hostID(hostID), m_peer(destination)
        {}

        int GetHostID() const { return m_hostID; }
        ENetPeer* GetPeer() const { return m_peer; }

    private:
        const int m_hostID;
        ENetPeer* m_peer; // Only used when sending direct. Is not translated.
    };


    /**
     * @brief A packet for sending user data over the network.
     * 
     * How the packet is handled depends upon the LobbyAction provided.
     * JOIN - Adds the user to the local lobby.
     * CREATE - Usually a response from a RequestUserIDPacket
     * LEAVE - Removes the user from the local lobby.
     */
    class UserInfoPacket : public Packet {
    public:
        UserInfoPacket(User user, LobbyAction action) :
            Packet(static_cast<Type>(PacketType::USER_INFO), static_cast<uint8_t>(Channel::RELIABLE), 0),
            m_user(user), m_action(action)
        {}

        User GetUser() const { return m_user; }
        LobbyAction GetAction() const { return m_action; }

    private:
        const User m_user;
        const LobbyAction m_action;
    };


    /**
     * @brief A simple packet for requesting a unique user id from the server.
     */
    class RequestUserIDPacket : public Packet {
    public:
        /**
         * @brief Constructor for RequestUserIDPacket.
         * @param destination - nullptr for broadcasting, otherwise, direct.
         * Used for responding.
         */
        RequestUserIDPacket(ENetPeer* destination) :
            Packet(static_cast<Type>(PacketType::REQUEST_USERID), static_cast<uint8_t>(Channel::UNSEQUENCED), 0),
            m_peer(destination)
        {}

        ENetPeer* GetPeer() const { return m_peer; }

    private:
        ENetPeer* m_peer;
    };
}

