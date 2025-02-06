#pragma once
#include "Transform.h"

#include "PhysicsObject.h"
#include "btBulletDynamicsCommon.h"

using std::vector;

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

		virtual void OnCollisionEnter(GameObject* otherObject) {
			std::cout << "OnCollisionBegin event occured! " << this->GetWorldID() << " " << otherObject->GetWorldID() << std::endl;
		}

		virtual void OnCollisionExit(GameObject* otherObject) {
			std::cout << "OnCollisionEnd event occured!\n";
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
			hasSetInitialPosition = true;
		}

		void setInitialRotation(const Quaternion& rotation) {
			initialRotation = rotation;
			hasSetInitialRotation = true;
		}

		Vector3 getInitialPosition() const {
			return initialPosition;
		}
		Quaternion getInitialRotation() const {
			return initialRotation;
		}

		Vector3 getRenderScale() const {
			return renderScale;
		}

		void setRenderScale(const Vector3& scale) {
			renderScale = scale;
		}

	protected:
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool		isActive;
		int			worldID;
		std::string	name;

		Vector3 renderScale; // Only affects rendering, not physics
		Vector3 initialPosition;
		Quaternion initialRotation;
		bool hasSetInitialPosition;
		bool hasSetInitialRotation;
	};
}
