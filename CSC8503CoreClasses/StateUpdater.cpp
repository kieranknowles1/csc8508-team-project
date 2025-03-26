
#include "StateUpdater.h"
#include "WorldState.h"

using namespace NCL::CSC8503;

StateUpdater::StateUpdater() {
    states = std::make_unique<WorldState::StateBuffer>();
}
