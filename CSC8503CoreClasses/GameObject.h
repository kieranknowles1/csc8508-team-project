#pragma once

#include "PhysicsObject.h"
#include "btBulletDynamicsCommon.h"

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;

	class GameObject	{
	public:
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

		virtual void OnCollisionEnter(GameObject* otherObject, const btVector3& contactPointA, const btVector3& contactPointB) {
			//std::cout << "OnCollisionBegin event occured! " << this->GetWorldID() << " " << otherObject->GetWorldID() << std::endl;
		}

		virtual void OnCollisionExit(GameObject* otherObject) {
		//	std::cout << "OnCollisionEnd event occured!\n";
		}

		virtual void Update(float dt) {

		}

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
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

		btVector3 getInitialPosition() const {
			return initialPosition;
		}
		btQuaternion getInitialRotation() const {
			return initialRotation;
		}

		Vector3 getRenderScale() const {
			return renderScale;
		}

		void setRenderScale(const Vector3& scale) {
			renderScale = scale;
		}
		void setIsFloor(bool floorIn) {
			floor = floorIn;
		}
		bool getIsFloor() {
			return floor;
		}

	protected:
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool		isActive;
		bool floor = false;
		int			worldID;
		std::string	name;

		Vector3 renderScale = Vector3(1, 1, 1); // Only affects rendering, not physics
		btVector3 initialPosition;
		btQuaternion initialRotation = btQuaternion(0, 0, 0);
	};
}
