#include "NavEntity.h"
#include <stdio.h>

using namespace NCL;
using namespace CSC8503;

void NavEntity::NewPath(std::vector<btVector3> newPath) {
	path = newPath;
	nextNode = 0;
	curPathPoint = path[0];
}

bool NavEntity::FollowPath(float dt, GameObject* player) {
	
	if (nextNode == -1) return false;

	btVector3 nextPoint = path[nextNode];

	float dist = curPathPoint.distance(nextPoint);
	btVector3 dir = nextPoint - curPathPoint;
	btVector3 toPoint = (dir.length2() == 0) ? btVector3(0, 0, 0) : dir.normalized();


	if (speed <= dist) {
		newPathPoint = curPathPoint + (toPoint * speed);
	}
	else {
		if ((++nextNode) == path.size()) {
			newPathPoint = nextPoint;
			nextNode = -1;
		}
		else {
			float remainder = speed - dist;
			btVector3 remainderDir = ((path[nextNode] - nextPoint).normalize()) * remainder;
			newPathPoint = nextPoint + remainderDir;
		}
	}
	curPathPoint.setY(curPathPoint.getY() - YAdjust(curPathPoint, player));
	curPathPoint = newPathPoint;
	return true;
}

float NavEntity::YAdjust(btVector3 pos, GameObject* player) {
	btVector3 upPos = pos + btVector3(0, 5.0f, 0);
	btVector3 downPos = pos + btVector3(0, -10000.0f, 0);
	btCollisionWorld::AllHitsRayResultCallback callback(upPos, downPos);
	float smallestDist = INFINITY;
	btVector3 hitPoint = btVector3();
	if (callback.hasHit()) {
		GameObject* hitObj = nullptr;
		for (int i = 0; i < callback.m_collisionObjects.size(); i++) { // loop all hits
			GameObject* hit = static_cast<GameObject*>(callback.m_collisionObjects[i]->getUserPointer());
			if (!hit->getIsPaintball() && hit != player) { // ignore paintballs
				btVector3 posHit = callback.m_hitPointWorld[i];
				float distance = pos.distance(posHit);
				if (distance < smallestDist) { // find closest valid hit
					smallestDist = distance;
					hitPoint = posHit;
					hitObj = hit;
				}
			}
		}
	}
	return smallestDist;
}