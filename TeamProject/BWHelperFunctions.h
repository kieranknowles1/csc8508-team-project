#pragma once

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include "GameObject.h"

using namespace std;
namespace NCL::CSC8503 {
	class BWHelperFunctions {
	public:
		BWHelperFunctions(btDiscreteDynamicsWorld* world);
		pair<btVector3, GameObject*> Raycast(const btVector3& start, const btVector3& end);
		vector<pair<btVector3, GameObject*>> RaycastAll(const btVector3& start, const btVector3& end);
	private:
		btDiscreteDynamicsWorld* world;
	};
}