#include "NavEntity.h"
#include <stdio.h>

using namespace NCL;
using namespace CSC8503;

void NavEntity::NewPath(std::vector<btVector3> newPath) {
	path = newPath;
	nextNode = 0;
}

void NavEntity::FollowPath(float dt) {
	
	if (nextNode != -1) {
		trans = GetTransform();
		btVector3 location = trans.getOrigin();
		btVector3 nextPoint = path[nextNode];

		float dist = location.distance(nextPoint);
		btVector3 dir = nextPoint - location;
		btVector3 toPoint = (dir.length2() == 0) ? btVector3(0, 0, 0) : dir.normalized();

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
				btVector3 remainderDir = ((path[nextNode] - nextPoint).normalize()) * remainder;
				pathPoint = nextPoint + remainderDir;
			}
		}
		//trans.setOrigin(newLocation);
		//btRigidBody* body = physicsObject->GetRigidBody();
		//body->setWorldTransform(trans);
	}
}