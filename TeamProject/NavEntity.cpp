#include "NavEntity.h"
#include "TutorialGame.h"
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

	float frameSpeed = speed * dt;
	if (frameSpeed <= dist) {
		newPathPoint = curPathPoint + (toPoint * frameSpeed);
	}
	else {
		if ((++nextNode) == path.size()) {
			newPathPoint = nextPoint;
			nextNode = -1;
		}
		else {
			float remainder = frameSpeed - dist;
			btVector3 remainderDir = ((path[nextNode] - nextPoint).length2() == 0 ? btVector3(0, 0, 0) : ((path[nextNode] - nextPoint).normalize())) * remainder;
			newPathPoint = nextPoint + remainderDir;
		}
	}
	curPathPoint = newPathPoint;
	yAdjustedPoint = curPathPoint;
	//yAdjustedPoint.setY(GroundAdjust(yAdjustedPoint));
	return true;
}

float NavEntity::GroundAdjust(btVector3 pos) {
	btVector3 upPos, downPos, direction;
	std::optional<ShotInfo> rayResult;
	btIDebugDraw* debugDrawer = TutorialGame::getInstance()->getBulletWorld()->getDebugDrawer();
	switch (side) {
	case(Side::BOTTOM):
		upPos = pos + btVector3(0, 100.0f, 0);
		downPos = pos + btVector3(0, -1000.0f, 0);
		
		debugDrawer->drawLine(upPos, downPos, Vector3(0, 0, 1));
		direction = (downPos - upPos).normalized();

		rayResult = Shoot::GetInstance()->RayClosest(upPos, direction, this);

		if (!rayResult.has_value()) {
			return 0.0f;  // No hit detected
		}

		//return (pos.getY() - rayResult->hitPos.getY());
		return rayResult->hitPos.getY();
	default:
		return pos.getY();
	}
}
