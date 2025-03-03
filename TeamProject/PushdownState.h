#pragma once
#include "GameTechRendererInterface.h"

namespace NCL {
	namespace CSC8503 {
		class PushdownState {
		public:
			enum PushdownResult {
				Push, Pop, NoChange
			};
			PushdownState() {}
			virtual ~PushdownState() {}

			virtual PushdownResult OnUpdate(float dt, PushdownState** pushFunc) = 0;
			virtual void OnAwake() {}
			virtual void OnSleep() {}

			virtual void OnRender(GameTechRendererInterface* renderer) {}
		};
	}
}
