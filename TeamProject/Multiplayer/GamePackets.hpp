#pragma once

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include "GameObject.h"

#ifdef BUILD_PRE

#include "../Network/Packet.hpp"
#include "../Network/Network.hpp"
#include "../TeamProject/Multiplayer/User.hpp"
#include "../TeamProject/Multiplayer/Lobby.hpp"

#else
#include "Network/Packet.hpp"
#include "Network/Network.hpp"
#include "Multiplayer/User.hpp"
#include "Multiplayer/Lobby.hpp"
#endif

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
        REQUEST_USERID = CUSTOM_TYPE + 6,
        DAMAGE = CUSTOM_TYPE + 7,
        LASER = CUSTOM_TYPE + 8,
        PING = CUSTOM_TYPE + 9,
        PONG = CUSTOM_TYPE + 10,
        SCORE = CUSTOM_TYPE + 11
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
    * @brief Laser Packet class.
    *
    * Stores the start and end positions of a laser.
    * Used by clients for displaying other players lasers.
    */
    class LaserPacket : public Packet {
    public:
        LaserPacket(int objectID, const btVector3& startPos, const btVector3& endPos, const btVector3& hitNormal, int sequenceNum) :
            Packet(static_cast<Type>(PacketType::LASER), static_cast<int>(Channel::FREQUENT), sequenceNum),
            m_objectID(objectID), m_startPos(startPos), m_endPos(endPos), m_hitNormal(hitNormal)
        {}

        inline int GetTargetID() const { return m_objectID; }
        inline btVector3 GetStartPos() const { return m_startPos; }
        inline btVector3 GetEndPos() const { return m_endPos; }
        inline btVector3 GetHitNormal() const { return m_hitNormal; }

    private:
        const int m_objectID;
        const btVector3 m_startPos;
        const btVector3 m_endPos;
        const btVector3 m_hitNormal;
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
         * @return btVector3 containing the new position data.
         */
        inline btVector3 GetPosition() const { return m_position; }

        /**
         * @brief Get the new orientation of the game object from the packet.
         * @return btQaternion containing the new orientation data. 
         */
        inline btQuaternion GetOrientation() const { return m_orientation; }

    private:
        const int m_objectID;
        const btVector3 m_position;
        const btQuaternion m_orientation;
    };


    ///**
    // * @brief Player Change State Packet class.
    // * 
    // * Used to notify of a player's state change (usually spawning in or
    // * dying).
    // */
    //class PlayerChangeStatePacket : public Packet {
    //public:
    //    PlayerChangeStatePacket(int playerID, const ObjectState& state) :
    //        Packet(static_cast<Type>(PacketType::PLAYER_STATE_CHANGE), static_cast<int>(Channel::UNSEQUENCED), 0),
    //        m_playerID(playerID), m_newState(state)
    //    {}

    //    int GetPlayerID() const { return m_playerID; }
    //    ObjectState GetState() const { return m_newState; }

    //private:
    //    int m_playerID;
    //    ObjectState m_newState;
    //};


    /**
     * @brief A packet that updates an objects upVector to change its gravity.
     */
    class ObjectChangeGravityPacket : public Packet {
    public:
        ObjectChangeGravityPacket(int objectID, const btVector3& upDirection, uint32_t sequenceNum) :
            Packet(static_cast<Type>(PacketType::OBJECT_CHANGE_GRAVITY), static_cast<int>(Channel::FREQUENT), sequenceNum),
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
     * @brief A packet for sending user data over the network.
     * 
     * How the packet is handled depends upon the LobbyAction provided.
     * JOIN - Adds the user to the local lobby.
     * CREATE - Usually a response from a RequestUserIDPacket
     * LEAVE - Removes the user from the local lobby.
     */
    class UserInfoPacket : public Packet {
    public:
        UserInfoPacket(Lobbies::User user, Lobbies::LobbyAction action) :
            Packet(static_cast<Type>(PacketType::USER_INFO), static_cast<uint8_t>(Channel::RELIABLE), 0),
            m_user(user), m_action(action)
        {}

        Lobbies::User GetUser() const { return m_user; }
        Lobbies::LobbyAction GetAction() const { return m_action; }

    private:
        const Lobbies::User m_user;
        const Lobbies::LobbyAction m_action;
    };


    /**
     * @brief Simple packet for sending damage across the network.
     */
    class DamagePacket : public Packet {
    public:
        /**
         * @brief Constructor for the DamagePacket.
         * @param targetID - the id of the target to damage.
         * @param damage - the amount of damage to deal.
         * @param dealer - the id of the user who dealt the damage.
         */
        DamagePacket(int targetID, float damage, int dealer) :
            Packet(static_cast<Type>(PacketType::DAMAGE), static_cast<uint8_t>(Channel::UNSEQUENCED), 0),
            m_targetID(targetID), m_damage(damage), m_dealer(dealer)
        {}

        inline int GetTargetID() const { return m_targetID; }
        inline float GetDamage() const { return m_damage; }
        inline int GetDamageDealer() const { return m_dealer; }

    private:
        int m_targetID;
        float m_damage;
        int m_dealer;
    };


    /**
     * @brief Ping the server or client.
     */
    class PingPacket : public Packet {
    public:
        PingPacket() :
            Packet(static_cast<Type>(PacketType::PING), static_cast<uint8_t>(Channel::RELIABLE), 0)
        {}
    };


    /**
     * @brief Pong a ping.
     */
    class PongPacket : public Packet {
    public:
        PongPacket() :
            Packet(static_cast<Type>(PacketType::PONG), static_cast<uint8_t>(Channel::RELIABLE), 0)
        {}
    };
    
    
    /**
     * @brief Score packet for updating scores for players.
     */
    class ScorePacket : public Packet {
    public:
        ScorePacket(int objectID, float score, int sequenceNum) :
            Packet(static_cast<Type>(PacketType::SCORE), static_cast<uint8_t>(Channel::UNSEQUENCED), sequenceNum),
            m_objectID(objectID), m_score(score)
        {}

        int GetObjectID() const { return m_objectID; }
        float GetScore() const { return m_score; }

    private:
        int m_objectID;
        float m_score;
    };
}

