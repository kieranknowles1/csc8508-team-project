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
  return make_pair(btVector3(0,0,0), nullptr);
}

//Returns the vector of hits and an empty vector if there are no hits
vector<pair<btVector3, GameObject*>> BWHelperFunctions::RaycastAll(const btVector3& start, const btVector3& end) {
    btCollisionWorld::AllHitsRayResultCallback callback(start, end);
    world->rayTest(start, end, callback);

    vector<pair<btVector3, GameObject*>> hits;

    if (callback.hasHit()) {
        for (int i = 0; i < callback.m_collisionObjects.size(); i++) {
            const btRigidBody* hitBody = btRigidBody::upcast(callback.m_collisionObjects[i]);
            if (hitBody) {
                GameObject* hitObj = static_cast<GameObject*>(hitBody->getUserPointer());
                hits.emplace_back(callback.m_hitPointWorld[i], hitObj);
            }
        }
    }

    return hits;
}