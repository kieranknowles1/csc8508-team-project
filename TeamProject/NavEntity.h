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
			bool FollowPath(float dt);
			btVector3 newPathPoint;
			btVector3 curPathPoint;
			btVector3 yAdjustedPoint;
			float GroundAdjust(btVector3 pos);
			char side;
			float speed = 20;
		private:


			std::vector<btVector3> path = {};
			int nextNode = -1;

			btTransform trans;
		};
	}
}