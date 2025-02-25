#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class NavEntity : public GameObject {
		public:
			NavEntity() {};
			~NavEntity() {};

			void NewPath(std::vector<btVector3> newPath);
			void Update(float dt);
		private:
			float speed = 0.01;

			std::vector<btVector3> path = {};
			int nextNode = -1;

			btTransform trans;
		};
	}
}