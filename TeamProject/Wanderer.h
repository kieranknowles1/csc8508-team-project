#pragma once
#include "NavEntity.h"

namespace NCL {
	namespace CSC8503 {
		class Wanderer : public NavEntity {
		public:
			Wanderer() {};
			~Wanderer() {};

			void Update(float dt);

		private:

		};
	}
}