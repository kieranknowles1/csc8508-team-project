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
	float yaw;
};

class Respawn {
public:
	Respawn() { instance = this; }
	~Respawn() {};
	static Respawn* GetInstance() { return instance; }

	void InsertRespawn(RespawnPoint* respawnPoint) {
		respawnPoints.push_back(respawnPoint);
	}

	RespawnPoint* GetRespawn(unsigned int ID) {
		if (respawnPoints.size() < ID) {
			std::cerr << "RESPAWN POINT NOT FOUND" << std::endl;
			return nullptr;
		}
		return respawnPoints.at(ID);
	}


	RespawnPoint* GetRandomRespawn() {
		std::cout << "RESPAWNS: " << respawnPoints.size() << std::endl;
		int ID = rand() % respawnPoints.size();
		return respawnPoints.at(ID);
	}

private:
	inline static Respawn* instance = nullptr;
	std::vector<RespawnPoint*> respawnPoints;

};