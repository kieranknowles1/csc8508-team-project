#pragma once

#include <memory>
#include <vector>
#include "WorldState.h"

namespace Packet {
	class Packet;
}

namespace NCL {
	namespace CSC8503 {
		class StateUpdater {
		public:
			StateUpdater() {};

			virtual void UpdateWorldState() = 0;
			virtual void UpdateFromWorldState(float dt) = 0;
			virtual std::vector<std::shared_ptr<Packet::Packet>> CreatePackets(int sequenceNum) = 0;

			WorldState::StateBuffer* GetWorldStates() { return &states; }

		protected:
            float elapsedTickTime = 0;

		private:
			WorldState::StateBuffer states;
		};
	}
}