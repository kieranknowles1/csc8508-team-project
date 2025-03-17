#pragma once

#include <mutex>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "PhysicsObject.h"
#include "btBulletDynamicsCommon.h"
#include "CollisionInfo.h"
#include "../TeamProject/PointLight.h"
#include "../TeamProject/Multiplayer/User.hpp"
#include "../TeamProject/Multiplayer/WorldState.hpp"

namespace NCL::CSC8503 {
    class NetworkObject;
    class RenderObject;
    class PhysicsObject;

	class GameObject	{
	public:
		enum class Type {
			Default,
			Floor,
			JumpPad,
			Slime,
			Ice,
			PointLight,
			RespawnPoint,
			Player,
			Centre,
			SlimeCastle,
			Courtyard
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
            GetPhysicsObject()->GetRigidBody()->setLinearVelocity(std::get<btVector3>(objectWorldState.ReadState(WorldState::StateType::LinearVelocity)));
            GetPhysicsObject()->GetRigidBody()->setAngularVelocity(std::get<btVector3>(objectWorldState.ReadState(WorldState::StateType::AngularVelocity)));
            GetPhysicsObject()->GetRigidBody()->getWorldTransform().setOrigin(std::get<btVector3>(objectWorldState.ReadState(WorldState::StateType::Position)));
            GetPhysicsObject()->GetRigidBody()->getWorldTransform().setRotation(std::get<btQuaternion>(objectWorldState.ReadState(WorldState::StateType::Rotation)));

            std::vector<std::pair<WorldState::StateType, WorldState::StateValue>> stateUpdates;
            stateUpdates.push_back(
                { WorldState::StateType::LinearVelocity, GetPhysicsObject()->GetRigidBody()->getLinearVelocity() }
            );
            stateUpdates.push_back(
                { WorldState::StateType::AngularVelocity, GetPhysicsObject()->GetRigidBody()->getAngularVelocity() }
            );
            stateUpdates.push_back(
                { WorldState::StateType::Position, GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin() }
            );
            stateUpdates.push_back(
                { WorldState::StateType::Rotation, GetPhysicsObject()->GetRigidBody()->getWorldTransform().getRotation() }
            );
            objectWorldState.UpdateStates(stateUpdates);
        }

        void SetWorldID(int newID) {
            worldID = newID;
            objects[worldID] = this;
        }

        int		GetWorldID() {
            return worldID;
        }

        btTransform GetTransform() const {
            return physicsObject->GetRigidBody()->getWorldTransform();
        }

        void setInitialPosition(const Vector3& position) {
            initialPosition = position;
        }

        void setInitialRotation(const btQuaternion& rotation) {
            initialRotation = rotation;
        }

        btVector3 getInitialPosition() {
            return initialPosition;
        }
        btQuaternion getInitialRotation() {
            return initialRotation;
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

        WorldState::ObjectState& GetWorldState() { return objectWorldState; }

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
        WorldState::ObjectState objectWorldState;
    };
}
