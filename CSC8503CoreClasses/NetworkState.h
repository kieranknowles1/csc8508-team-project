#pragma once

#include <LinearMath/btQuaternion.h>

namespace NCL {
	using namespace Maths;
	namespace CSC8503 {
		class NetworkState	{
		public:
			NetworkState();
			virtual ~NetworkState();

			Vector3		position;
			btQuaternion orientation;
			int			stateID;
		};
	}
}

