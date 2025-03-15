#pragma once

#include <mutex>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "PhysicsObject.h"
#include "btBulletDynamicsCommon.h"
#include "CollisionInfo.h"
#include "../TeamProject/PointLight.h"
#include "../TeamProject/Multiplayer/User.hpp"

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;

	class GameObject	{
	public:
		enum class Type {
			Default,
			JumpPad,
			Slime,
			Ice,
			PointLight,
			RespawnPoint,
			Player
		};

		GameObject(const std::string& name = "");
		virtual ~GameObject();

		bool IsActive() const {
			return isActive;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		void SetName( const std::string& nameIn)  {
			name = nameIn;
		}

		virtual void OnCollisionEnter(const CollisionInfo& collision) {
			//std::cout << "OnCollisionEnter event occured!\n";
		}

		virtual void OnCollisionExit(const CollisionInfo& collision) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		virtual void OnCollisionStay(const CollisionInfo& collision) {
			//std::cout << "OnCollisionStay event occured!\n";
		}

		// TODO:: Remove this OnCollisionStay method, since the CollisionInfo returns the otherObject now in the above function
		virtual void OnCollisionStay(GameObject* otherObject) {
			//std::cout << "OnCollisionStay: " << this->GetWorldID() << " is still colliding with " << otherObject->GetWorldID() << std::endl;
		}

		virtual void Update(float dt) {

		}

		void SetWorldID(int newID) {
			std::lock_guard<std::mutex> lock(m_stateLock);
			worldID = newID;
			objects[worldID] = this;
		}

		int		GetWorldID() {
			std::lock_guard<std::mutex> lock(m_stateLock);
			return worldID;
		}

		btTransform GetTransform() const {
			return physicsObject->GetRigidBody()->getWorldTransform();
		}

		void setInitialPosition(const Vector3& position) {
			std::lock_guard<std::mutex> lock(m_stateLock);
			initialPosition = position;
		}

		void setInitialRotation(const btQuaternion& rotation) {
			std::lock_guard<std::mutex> lock(m_stateLock);
			initialRotation = rotation;
		}

		btVector3 getInitialPosition() {
			std::lock_guard<std::mutex> lock(m_stateLock);
			return initialPosition;
		}
		btQuaternion getInitialRotation() {
			std::lock_guard<std::mutex> lock(m_stateLock);
			return initialRotation;
		}

		btVector3 getLinearVelocity() {
			btRigidBody* rigidBody = GetPhysicsObject()->GetRigidBody();
			std::lock_guard<std::mutex> lock(m_stateLock);
			return rigidBody->getLinearVelocity();
		}

		btVector3 getAngularVelocity() {
			btRigidBody* rigidBody = GetPhysicsObject()->GetRigidBody();
			std::lock_guard<std::mutex> lock(m_stateLock);
			return rigidBody->getAngularVelocity();
		}

		btVector3 getPosition() {
			btTransform& transform = GetPhysicsObject()->GetRigidBody()->getWorldTransform();
			std::lock_guard<std::mutex> lock(m_stateLock);
			return transform.getOrigin();
		}

		btQuaternion getRotation() {
			btTransform& transform = GetPhysicsObject()->GetRigidBody()->getWorldTransform();
			std::lock_guard<std::mutex> lock(m_stateLock);
			return transform.getRotation();
		}

		Vector3 getRenderScale() const {
			return renderScale;
		}

		void SetOwner(Lobbies::User user) {
			owner.emplace(user);
		}

		std::optional<Lobbies::User> GetOwner() {
			return owner;
		}

		void setRenderScale(const Vector3& scale) {
			renderScale = scale;
		}
		void setIsPaintball(bool paintballIn) {
			paintball = paintballIn;
		}
		bool getIsPaintball() {
			return paintball;
		}
		void setType(Type typeIn) {
			type = typeIn;
		}
		Type getType() {
			return type;
		}

		bool isStatic() const {
			return GetPhysicsObject()->GetRigidBody()->getInvMass() == 0.0f;
		}

		static GameObject* GetGameObjectByID(int id) {
			if (objects.contains(id)) return objects[id];
			return nullptr;
		}

		void attachLight(PointLight* lightIn) {
			light = lightIn;
		}
		PointLight* getLight() {
			return light;
		}

		int GetLastPacketSequence(uint8_t type) {
			if (lastPacketUpdates.contains(type)) return lastPacketUpdates[type];
			else return 0;
		}

		void UpdatePacketSequence(uint8_t type, int value) {
			lastPacketUpdates[type] = value;
		}


		void setDeleted() { deleted = true; }
		bool isDeleted() { return deleted; }
	protected:
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool deleted = false;

		bool		isActive;
		bool paintball = false;
		int			worldID;
		std::string	name;
		Type type;

		Vector3 renderScale = Vector3(1, 1, 1); // Only affects rendering, not physics
		btVector3 initialPosition;
		btQuaternion initialRotation = btQuaternion(0, 0, 0);

		inline static std::unordered_map<int, GameObject*> objects;

		PointLight* light = nullptr;

		std::optional<Lobbies::User> owner;
		std::unordered_map<uint8_t, int> lastPacketUpdates; // uint8_t is the same type used in Packet::Type and PacketType

		std::mutex m_stateLock;
	};
}
