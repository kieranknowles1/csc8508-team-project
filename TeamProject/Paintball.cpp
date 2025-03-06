#include "Paintball.h"
#include "TutorialGame.h"

void Paintball::OnCollisionEnter(const CollisionInfo& collisionInfo){
	if (collisionInfo.otherObject == player) return;
	TutorialGame::getInstance()->delayedRemoveObject(this);
}