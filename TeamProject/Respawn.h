#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"
#include "PhysicsObject.h"
#include "CollisionInfo.h"

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

using namespace NCL::CSC8503;

struct RespawnPoint {
	btVector3 position;
	btVector3 orientation;
};

class Respawn {
public:
	Respawn() { instance = this; }
	~Respawn();
	static Respawn* GetInstance() { return instance; }

	void InsertRespawn(RespawnPoint* respawnPoint) {
		respawnPoints.push_back(respawnPoint);
	}

	RespawnPoint* GetRespawn(unsigned int ID) {
		return respawnPoints.at(ID);
	}

private:
	inline static Respawn* instance = nullptr;
	std::vector<RespawnPoint*> respawnPoints;

};