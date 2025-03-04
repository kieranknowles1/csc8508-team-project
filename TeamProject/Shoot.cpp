#include "Shoot.h"

using namespace NCL;
using namespace CSC8503;


GameObject* Shoot::ShootBullet(btVector3 startPos, btVector3 dir) {
	GameObject* closetHit = nullptr;
	dir.normalize();
	std::cout << "SHOOTING YAY" << std::endl;

	return closetHit;
}