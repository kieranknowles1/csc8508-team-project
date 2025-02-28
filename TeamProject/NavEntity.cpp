#include "NavEntity.h"
#include <stdio.h>

using namespace NCL;
using namespace CSC8503;

void NavEntity::NewPath(std::vector<btVector3> newPath) {
	path = newPath;
	nextNode = 0;
}

bool NavEntity::FollowPath(btVector3 location, float dt) {
	
	if (nextNode == -1) return false;

	btVector3 nextPoint = path[nextNode];

	float dist = location.distance(nextPoint);
	btVector3 dir = nextPoint - location;
	btVector3 toPoint = (dir.length2() == 0) ? btVector3(0, 0, 0) : dir.normalize();

	if (speed <= dist) {
		pathPoint = location + (toPoint * speed);
	}
	else {
		if ((++nextNode) == path.size()) {
			pathPoint = nextPoint;
			nextNode = -1;
		}
		else {
			float remainder = speed - dist;
			btVector3 diff = path[nextNode] - nextPoint;
			btVector3 remainderDir = (diff.length2() == 0) ? dir : (diff.normalize() * remainder);
			pathPoint = nextPoint + remainderDir;
		}
	}
	return true;
}