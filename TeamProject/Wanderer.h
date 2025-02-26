#pragma once
#include "NavEntity.h"
#include "NavMesh.h"

namespace NCL {
	namespace CSC8503 {
		class Wanderer : public NavEntity {
		public:
			Wanderer(NavMesh* nav);
			~Wanderer() {};

			void Update(float dt);
			void SetOffset();

		private:
			NavMesh* navMesh;
			std::vector<btVector3> curPath = {};
			btVector3 navOffset;
		};
	}
}