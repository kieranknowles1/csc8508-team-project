#include "NavEntity.h"
#include <stdio.h>

using namespace NCL;
using namespace CSC8503;

void NavEntity::NewPath(std::vector<btVector3> newPath) {
	path = newPath;
	nextNode = 0;
	curPathPoint = path[0];
}

bool NavEntity::FollowPath(float dt) {
	
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
			btVector3 remainderDir = ((path[nextNode] - nextPoint).length2() == 0 ? btVector3(0, 0, 0) : ((path[nextNode] - nextPoint).normalize())) * remainder;
			newPathPoint = nextPoint + remainderDir;
		}
	}
	curPathPoint = newPathPoint;
	yAdjustedPoint = curPathPoint;
	yAdjustedPoint.setY(yAdjustedPoint.getY() - YAdjust(yAdjustedPoint));
	return true;
}

float NavEntity::YAdjust(btVector3 pos) {
	btVector3 upPos = pos + btVector3(0, 5.0f, 0);
	btVector3 downPos = pos + btVector3(0, -10000.0f, 0);
	btVector3 direction = (downPos - upPos).normalized();
	ShotInfo* rayResult = Shoot::GetInstance()->RayClosest(upPos, direction, this);
	if (!rayResult) return 0;
	return (pos.getY() - rayResult->hitPos.getY());
}