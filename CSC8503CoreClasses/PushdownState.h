#pragma once

namespace NCL {
	namespace CSC8503 {
		class PushdownState
		{
		public:
			enum PushdownResult {
				// Push a new state, held in the newState output
				Push,
				// Pop a state from the stack, exiting if the top state was popped
				Pop,
				// Do not change state
				NoChange,
				// Remove all states, except for the topmost one
				Clear
			};
			PushdownState()  {
			}
			virtual ~PushdownState() {}

			virtual PushdownResult OnUpdate(float dt, PushdownState** pushFunc) = 0;
			virtual void OnAwake() {}
			virtual void OnSleep() {}
			//virtual std::string GetState() {};
		protected:
		};
	}
}
