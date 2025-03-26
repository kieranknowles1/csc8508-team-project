
#include "StateUpdater.h"

using namespace NCL::CSC8503;
using namespace WorldState;

StateUpdater::StateUpdater() {
    states = std::make_unique<StateBuffer>();
}
