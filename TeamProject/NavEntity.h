#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class NavEntity : public GameObject {
		public:
			NavEntity();
			~NavEntity();
		private:
			float speed = 3;

			std::vector<btVector3> path;
			btVector3 nextNode;
		};
	}
}