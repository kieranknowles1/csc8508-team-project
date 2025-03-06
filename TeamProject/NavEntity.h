#pragma once
#include "GameObject.h"
#include "Shoot.h"

namespace NCL {
	namespace CSC8503 {
		class NavEntity : public GameObject {
		public:
			NavEntity() {};
			~NavEntity() {};

		protected:

			void NewPath(std::vector<btVector3> newPath);
			bool FollowPath(float dt, GameObject* player);
			btVector3 newPathPoint;
			btVector3 curPathPoint;
		private:
			float YAdjust(btVector3 pos, GameObject* player);

			float speed = 1;

			std::vector<btVector3> path = {};
			int nextNode = -1;

			btTransform trans;
		};
	}
}