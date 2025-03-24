#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"
#include "PhysicsObject.h"
#include "CollisionInfo.h"
//#include "PlayerObject.h"

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

	RespawnPoint* GetRandomRespawn(int playerID) {
	/*	RespawnPoint* chosenSpawn = nullptr;
		float furthestDist = 0.0f;
		for (RespawnPoint* point : respawnPoints) {
			for (PlayerObject* player : players) {
				if (player->GetWorldID() == playerID) {
					continue;
				}
				float currentDist = point->position.distance(player->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin());
				if (currentDist > furthestDist) {
					furthestDist = currentDist;
					chosenSpawn = point;
				}
			}
		}
		return chosenSpawn;*/
		return respawnPoints.at(0);
	}

	//void InsertPlayerObj(PlayerObject* playerObj) {
		//players.push_back(playerObj);
	//}

private:
	inline static Respawn* instance = nullptr;
	std::vector<RespawnPoint*> respawnPoints;
	//std::vector<PlayerObject*> players;


};