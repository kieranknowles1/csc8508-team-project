#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"
#include "PhysicsObject.h"
#include "CollisionInfo.h"
#include "PlayerObject.h"

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

	void ClearPlayers() {
		players.clear();
	}

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
		RespawnPoint* furthestSpawn = respawnPoints.at(0);
		float minDist = 1000.0f;
		float furthestDist = 0.0f;
		std::vector<RespawnPoint*> chosenPoints;
		for (RespawnPoint* point : respawnPoints) {
			float closestPlayerDist = INFINITY;
			for (PlayerObject* player : players) {
				if (player->GetWorldID() == playerID) {
					continue;
				}
				float currentDist = point->position.distance(player->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin());
				if (currentDist < closestPlayerDist) {
					closestPlayerDist = currentDist;
				}
			}
			if (closestPlayerDist > furthestDist) {
				furthestDist = closestPlayerDist;
				furthestSpawn = point;
			}
			if (closestPlayerDist > minDist) {
				chosenPoints.push_back(point);
			}
		}
		if (chosenPoints.size() > 0) {
			return chosenPoints.at(rand() % chosenPoints.size()); // choose random from those far enough away
		}
		else {
			return furthestSpawn; //none within min requirement - choose furthest possible
		}
	}

	void InsertPlayerObj(PlayerObject* playerObj) {
		players.push_back(playerObj);
	}

	static std::vector<PlayerObject*> GetAllPlayers() {
		return instance->players;
	}

private:
	inline static Respawn* instance = nullptr;
	std::vector<RespawnPoint*> respawnPoints;
	std::vector<PlayerObject*> players;


};