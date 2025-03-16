#pragma once

#include <LinearMath/btVector3.h>
#include <shared_mutex>
#include <unordered_map>
#include <variant>
#include <vector>

namespace WorldState {
    using StateValue = std::variant<btVector3>;

    enum class StateType {
        LinearVelocity,
        AngularVelocity,
        Position,
        UpVector
    };

    /**
     * @brief A thread safe class for storing different states.
     */
    class ObjectState {
    public:
        ObjectState() {}
        ~ObjectState() {}

        /**
         * @brief Lock this object for reading if you want to read multiple
         * states at once without allowing writes.
         * @return A shared lock that will exist till destructed.
         */
        std::shared_lock<std::shared_mutex> GetReadLock() {
            return std::shared_lock(m_stateLock);
        }

        /**
         * @brief Update a state.
         * This function blocks all reading until it has finished.
         */
        void UpdateState(const std::pair<StateType, StateValue>& stateUpdate) {
            std::unique_lock lock(m_stateLock);
            m_states[stateUpdate.first] = stateUpdate.second;
        }

        /**
         * @brief Write multiple states updates at once.
         */
        void UpdateStates(std::vector<std::pair<StateType, StateValue>>& stateUpdates) {
            std::unique_lock lock(m_stateLock);
            for (const std::pair<StateType, StateValue>& stateUpdate:stateUpdates) {
                m_states[stateUpdate.first] = stateUpdate.second;
            }
        }

        template <typename T>
        T ReadState(StateType type) {
            std::shared_lock lock(m_stateLock);
            return m_states[type];
        }

    private:
        std::shared_mutex m_stateLock;
        std::unordered_map<StateType, StateValue> m_states;
    };
}
