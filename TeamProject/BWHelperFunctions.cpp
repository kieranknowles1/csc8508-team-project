#include "BWHelperFunctions.h"

using namespace NCL;
using namespace CSC8503;
using namespace std;

BWHelperFunctions::BWHelperFunctions(btDiscreteDynamicsWorld* world) {
	this->world = world;
}

//Returns pair containing first hit location hit game object
pair<btVector3, GameObject*> BWHelperFunctions::Raycast(const btVector3& start, const btVector3& end) {
	btCollisionWorld::ClosestRayResultCallback callback(start, end);
	world->rayTest(start, end, callback);
	if (callback.hasHit()) {
		const btRigidBody* hitBody = btRigidBody::upcast(callback.m_collisionObject);
		GameObject* hitObj = static_cast<GameObject*>(hitBody->getUserPointer());
		return make_pair(callback.m_hitPointWorld, hitObj);
	}
}